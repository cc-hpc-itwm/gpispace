// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/Client.hpp>

#include <iml/detail/option.hpp>
#include <iml/vmem/gaspi/pc/proto/message.hpp>
#include <iml/vmem/gaspi/pc/segment/segment.hpp>

#include <util-generic/boost/program_options/validators/nonexisting_path_in_existing_directory.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/syscall.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>

#include <sys/un.h>

#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>

namespace
{
  // \todo use rpc::

  void close_socket (int fd)
  {
    fhg::util::syscall::shutdown (fd, SHUT_RDWR);
    fhg::util::syscall::close (fd);
  }

  int open_socket (::boost::filesystem::path const& path)
  {
    int const sfd = fhg::util::syscall::socket (AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr;

    memset (&addr, 0, sizeof (addr));
    addr.sun_family = AF_UNIX;
    strncpy (addr.sun_path, path.string().c_str(), sizeof (addr.sun_path) - 1);

    try
    {
      fhg::util::syscall::connect (sfd, reinterpret_cast<struct sockaddr*> (&addr), sizeof (struct sockaddr_un));
    }
    catch (::boost::system::system_error const&)
    {
      close_socket (sfd);
      throw;
    }

    return sfd;
  }
}

namespace iml
{
  using namespace gpi::pc;

      Client::Client (::boost::filesystem::path const& socket_path)
      try
        : _socket (open_socket (socket_path))
      {}
      catch (...)
      {
        std::throw_with_nested
          ( std::runtime_error
              ("Failed to open IML communication socket '" + socket_path.string() + "'")
          );
      }

  namespace
  {
    namespace validators = fhg::util::boost::program_options;

    namespace option
    {
      constexpr auto const name_socket ("iml-vmem-socket");
      using type_socket = ::boost::filesystem::path;
      using validator_socket = validators::nonexisting_path_in_existing_directory;
    }
  }

  Client::Client (::boost::program_options::variables_map const& vm)
    : Client ( detail::require<option::type_socket, option::validator_socket>
                 (vm, option::name_socket)
             )
  {}

  ::boost::program_options::options_description Client::options()
  {
    ::boost::program_options::options_description options ("Virtual memory");

    options.add_options()
      ( option::name_socket
      , ::boost::program_options::value<option::validator_socket>()
        ->required()
      , "socket file to communicate with the virtual memory manager"
      );

    return options;
  }

  void Client::set_socket
    (::boost::program_options::variables_map& vm, option::type_socket value)
  {
    detail::set<option::type_socket, option::validator_socket>
      ( vm, option::name_socket, value
      , [] (option::type_socket const& x)
        {
          return x.string();
        }
      );
  }

      Client::~Client ()
      {
          stop (std::unique_lock<std::mutex> (_socket_and_shm_segments_guard));
      }

      void Client::stop (std::unique_lock<std::mutex> const&)
      {
        if (_socket != -1)
        {
          try
          {
            close_socket (_socket);
          }
          catch (::boost::system::system_error const&)
          {
            // ignore already closed/invalid socket
          }
          _socket = -1;

          _shm_segments.clear();
        }
      }

      template < typename RequestCategory
               , typename Reply
               , typename Request
               , typename ReplyCategory
               >
        Reply Client::communicate ( std::unique_lock<std::mutex> const& lock
                                 , std::string const& what
                                 , Request const& request
                                 )
      {
        gpi::pc::proto::message_t const rqst {RequestCategory (request)};

        // serialize
          std::stringstream sstr;
          ::boost::archive::binary_oarchive oa (sstr);
          oa & rqst;
          std::string const data (sstr.str());

        // send
        proto::header_t header;
        header.clear();
        header.length = data.size();

        if ( (fhg::util::syscall::write (_socket, &header, sizeof (header)) <= 0)
           || (fhg::util::syscall::write (_socket, data.c_str(), data.size()) <= 0)
           )
        {
          stop (lock);
          throw std::runtime_error ("could not send data");
        }

        // receive
        if (fhg::util::syscall::read (_socket, &header, sizeof (header)) <= 0)
        {
          stop (lock);
          throw std::runtime_error ("could not receive data header");
        }

        std::vector<char> buffer (header.length);
        if (fhg::util::syscall::read (_socket, &buffer[0], header.length) <= 0)
        {
          stop (lock);
          throw std::runtime_error ("could not receive data");
        }

        proto::message_t rply;
        // deserialize
        {
          std::stringstream sstr2 (std::string (buffer.begin(), buffer.end()));
          ::boost::archive::text_iarchive ia (sstr2);
          ia & rply;
        }

        try
        {
          return ::boost::get<Reply> (::boost::get<ReplyCategory> (rply));
        }
        catch (::boost::bad_get const&)
        {
          throw std::runtime_error
            ( what
            + " failed: "
            + ::boost::get<proto::error::error_t> (rply).detail
            );
        }
        catch (...)
        {
          stop (lock);
          throw;
        }
      }

      AllocationHandle
      Client::create_allocation ( SegmentHandle const& segment
                   , MemorySize total_size
                   )
      {
        proto::memory::alloc_t alloc_msg;
        alloc_msg.segment = segment;
        alloc_msg.size = total_size;

        return communicate < proto::memory::message_t
                           , proto::memory::alloc_reply_t
                           > ( std::unique_lock<std::mutex> (_socket_and_shm_segments_guard)
                             , "memory allocation"
                             , alloc_msg
                             )
          .handle;
      }

      void Client::delete_allocation (AllocationHandle allocation)
      {
        proto::memory::free_t rqst;
        rqst.handle = allocation;

        std::unique_lock<std::mutex> const lock (_socket_and_shm_segments_guard);

        auto const result
          ( communicate < proto::memory::message_t
                        , proto::error::error_t
                        , proto::memory::free_t
                        , proto::error::error_t
                        > (lock, "memory free", rqst)
          );

        if (result.code != proto::error::success)
        {
          throw std::runtime_error ("handle could not be free'd: " + result.detail);
        }
      }

      SegmentHandle Client::create_segment
        ( SegmentDescription const& description
        , unsigned long total_size
        )
      {
        return communicate < proto::segment::message_t
                           , proto::segment::add_reply_t
                           > ( std::unique_lock<std::mutex> (_socket_and_shm_segments_guard)
                             , "create segment"
                             , proto::segment::add_memory_t
                               {description, total_size}
                             )
          .get();
      }
      void Client::delete_segment (SegmentHandle segment)
      {
        auto const result
          ( communicate < proto::segment::message_t
                        , proto::error::error_t
                        , proto::segment::del_memory_t
                        , proto::error::error_t
                        > ( std::unique_lock<std::mutex> (_socket_and_shm_segments_guard)
                          , "delete segment"
                          , proto::segment::del_memory_t (segment)
                          )
          );

        if (!result.detail.empty())
        {
          throw std::runtime_error (result.detail);
        }
      }

      std::unordered_map<std::string, double>
      Client::transfer_costs (std::list<MemoryRegion> regions)
      {
        return communicate < proto::memory::message_t
                           , proto::memory::transfer_costs_t
                           > ( std::unique_lock<std::mutex> (_socket_and_shm_segments_guard)
                             , "getting transfer costs"
                             , proto::memory::get_transfer_costs_t
                                 (std::move (regions))
                             )
          .costs;
      }

      Client::AllocationInformation Client::stat
        (AllocationHandle allocation)
      {
        // \todo Deduplicate with `pointer()` by deleting there.
        auto const descriptor
          ( communicate < proto::memory::message_t
                        , proto::memory::info_reply_t
                        > ( std::unique_lock<std::mutex> (_socket_and_shm_segments_guard)
                          , "getting memory information"
                          , proto::memory::info_t {allocation}
                          )
            .descriptor
          );
        return {descriptor.size};
      }

      std::unordered_set<SegmentHandle> Client::existing_segments()
      {
        return communicate < proto::existing_segments
                           , std::unordered_set<SegmentHandle>
                           , proto::existing_segments
                           , std::unordered_set<SegmentHandle>
                           > ( std::unique_lock<std::mutex> (_socket_and_shm_segments_guard)
                             , "getting existing segments"
                             , proto::existing_segments{}
                             );
      }
      std::unordered_set<AllocationHandle> Client::existing_allocations
        (SegmentHandle segment)
      {
        return communicate < proto::existing_allocations
                           , std::unordered_set<AllocationHandle>
                           , proto::existing_allocations
                           , std::unordered_set<AllocationHandle>
                           > ( std::unique_lock<std::mutex> (_socket_and_shm_segments_guard)
                             , "getting existing allocations of segment"
                             , proto::existing_allocations {segment}
                             );
      }

      char* Client::pointer (SharedMemoryAllocationHandle const& handle)
      {
        proto::memory::info_t rqst;
        rqst.handle = handle;

        std::unique_lock<std::mutex> const lock (_socket_and_shm_segments_guard);

        // \todo Offset is *always* 0 (only one allocation per shm
        // segment) so this remote query could be avoided, or cached.
        auto const descriptor
          ( communicate < proto::memory::message_t
                        , proto::memory::info_reply_t
                        > ( lock
                          , "getting memory information"
                          , rqst
                          )
            .descriptor
          );

        auto const seg_it (_shm_segments.find (handle));
        if (seg_it != _shm_segments.end())
        {
          return seg_it->second->ptr<char>() + descriptor.offset;
        }

        throw std::runtime_error
          ( ( ::boost::format("Requested pointer for unknown handle '%1%'")
            % handle
            ).str()
          );
      }

      MemcpyID Client::async_memcpy ( MemoryLocation const& destination
                                      , MemoryLocation const& source
                                      , MemorySize amount
                                      )
      {
        proto::memory::memcpy_t rqst;
        rqst.dst = destination;
        rqst.src = source;
        rqst.size = amount;

        return communicate < proto::memory::message_t
                            , proto::memory::memcpy_reply_t
                            > ( std::unique_lock<std::mutex> (_socket_and_shm_segments_guard)
                              , "memcpy"
                              , rqst
                              ).memcpy_id;
      }

      void Client::wait (MemcpyID memcpy_id)
      {
        proto::memory::wait_t rqst;
        rqst.memcpy_id = memcpy_id;

        return communicate < proto::memory::message_t
                           , proto::memory::wait_reply_t
                           > ( std::unique_lock<std::mutex> (_socket_and_shm_segments_guard)
                             , "wait"
                             , rqst
                             )
          .get();
      }

      void Client::memcpy ( MemoryLocation const& destination
                                  , MemoryLocation const& source
                                  , MemorySize amount
                                  )
      {
        wait (async_memcpy (destination, source, amount));
      }

      SharedMemoryAllocationHandle Client::create_shm_segment_and_allocate
        (MemorySize size)
      {
        static std::uint64_t shm_allocation_counter = 0;
        auto const name
          ( "/iml-"
          + std::to_string (fhg::util::syscall::getpid())
          + "-"
          + std::to_string (shm_allocation_counter++)
          );

        auto seg (std::make_shared<segment::segment_t> (name, size));

        try
        {
          seg->unlink();
        }
        catch (::boost::system::system_error const& se)
        {
          if (se.code ().value () == ENOENT)
          {
            // unlink() always throws, even  if the file did not
            // exist, so we have to ignore this error here.
          }
          else
          {
            throw;
          }
        }
        seg->create ();

        std::unique_lock<std::mutex> const lock (_socket_and_shm_segments_guard);

        proto::segment::register_t rqst;
        rqst.name = name;
        rqst.size = size;

        auto const result
          ( communicate < proto::segment::message_t
                        , proto::segment::register_reply_t
                        > ( lock
                          , "shm segment creation and allocation"
                          , rqst
                          ).handle
          );

        // TODO:
        //   what if we already have a segment with that id?
        //      maybe because the connection went down
        //      how and when is it safe to remove the segment?
        //      client code might be using the data in some way

        if (!_shm_segments.emplace (result, seg).second)
        {
          throw std::logic_error
            ( "Segment attached with id "
            + result.to_string()
            + " already exists"
            );
        }

        seg->unlink();

        return result;
      }

      void Client::free_and_delete_shm_segment
        (SharedMemoryAllocationHandle shared_memory_allocation)
      {
        proto::segment::unregister_t rqst;
        rqst.handle = shared_memory_allocation;

        std::unique_lock<std::mutex> const lock (_socket_and_shm_segments_guard);

        try
        {
          auto const result
            ( communicate < proto::segment::message_t
                          , proto::error::error_t
                          , proto::segment::unregister_t
                          , proto::error::error_t
                          > ( lock
                            , "delete segment"
                            , rqst
                            )
            );

          if (result.code != proto::error::success)
          {
            throw std::logic_error
              (std::to_string (result.code) + ": " + result.detail);
          }
        }
        catch (...)
        {
          throw std::runtime_error
            ( "could not unregister segment "
            + shared_memory_allocation.to_string()
            + ": " + fhg::util::current_exception_printer().string()
            );
        }

        // remove local
        _shm_segments.erase (shared_memory_allocation);
      }
}
