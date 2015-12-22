#include <gpi-space/pc/client/api.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/type/flags.hpp>

#include <fhg/assert.hpp>
#include <util-generic/syscall.hpp>
#include <fhglog/LogMacros.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>

#include <sys/un.h>

namespace
{
void close_socket (const int fd)
{
  fhg::util::syscall::shutdown (fd, SHUT_RDWR);
  fhg::util::syscall::close (fd);
}

int open_socket (std::string const & path)
{
  int const sfd = fhg::util::syscall::socket (AF_UNIX, SOCK_STREAM, 0);

  struct sockaddr_un addr;

  memset (&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy (addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

  try
  {
    fhg::util::syscall::connect (sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un));
  }
  catch (boost::system::system_error const& se)
  {
    close_socket (sfd);
    return -se.code().value();
  }

  return sfd;
}
}

namespace gpi
{
  namespace pc
  {
    namespace client
    {
      api_t::api_t ( fhg::log::Logger& logger
                   , std::string const & path
                   )
        : _logger (logger)
        , m_socket (open_socket (path))
      {}

      api_t::~api_t ()
      {
          stop ();
      }

      void api_t::stop ()
      {
        lock_type lock (m_mutex);

        if (m_socket != -1)
        {
          try
          {
            close_socket (m_socket);
          }
          catch (boost::system::system_error const &se)
          {
            // ignore already closed/invalid socket
          }
          m_socket = -1;

          m_segments.clear();
        }
      }

      gpi::pc::proto::message_t
      api_t::communicate(gpi::pc::proto::message_t const & rqst)
      {
        lock_type lock (m_mutex);

        // serialize
          std::stringstream sstr;
          boost::archive::binary_oarchive oa (sstr);
          oa & rqst;
          std::string const data (sstr.str());

        // send
        proto::header_t header;
        ::memset (&header, 0, sizeof(header));
        header.length = data.size();

        if ( (fhg::util::syscall::write (m_socket, &header, sizeof(header)) <= 0)
           || (fhg::util::syscall::write (m_socket, data.c_str(), data.size()) <= 0)
           )
        {
          stop ();
          throw std::runtime_error ("could not send data");
        }

        // receive
        if (fhg::util::syscall::read (m_socket, &header, sizeof (header)) <= 0)
        {
          stop ();
          throw std::runtime_error ("could not receive data header");
        }

        std::vector<char> buffer (header.length);
        if (fhg::util::syscall::read (m_socket, &buffer[0], header.length) <= 0)
        {
          stop ();
          throw std::runtime_error ("could not receive data");
        }

        proto::message_t rply;
        // deserialize
        {
          std::stringstream sstr2 (std::string(buffer.begin(), buffer.end()));
          boost::archive::text_iarchive ia (sstr2);
          ia & rply;
        }

        return rply;
      }

      type::handle_id_t
      api_t::alloc ( const type::segment_id_t seg
                   , const type::size_t sz
                   , const std::string & name
                   , const type::flags_t flags
                   )
      {
        proto::memory::alloc_t alloc_msg;
        alloc_msg.segment = seg;
        alloc_msg.size = sz;
        alloc_msg.name = name;
        alloc_msg.flags = flags;

        proto::message_t reply (communicate (proto::memory::message_t(alloc_msg)));

        try
        {
          proto::memory::message_t mem_msg (boost::get<proto::memory::message_t>(reply));
          proto::memory::alloc_reply_t alloc_rpl (boost::get<proto::memory::alloc_reply_t>(mem_msg));
          return alloc_rpl.handle;
        }
        catch (boost::bad_get const & ex)
        {
          proto::error::error_t error (boost::get<proto::error::error_t>(reply));
          throw std::runtime_error (error.detail);
        }
        catch (std::exception const & ex)
        {
          stop ();
          throw;
        }
      }

      void api_t::free (const gpi::pc::type::handle_id_t hdl)
      {
        proto::memory::free_t rqst;
        rqst.handle = hdl;

        proto::message_t rpl (communicate (proto::memory::message_t (rqst)));
        proto::error::error_t result (boost::get<proto::error::error_t>(rpl));
        if (result.code != proto::error::success)
        {
          throw std::runtime_error ("handle could not be free'd: " + result.detail);
        }
      }

      type::segment_id_t api_t::create_segment (std::string const& info)
      {
        proto::message_t const reply
          ( communicate ( proto::segment::message_t
                            (proto::segment::add_memory_t (info))
                        )
          );

        return boost::get<proto::segment::add_reply_t>
          (boost::get<proto::segment::message_t> (reply)).get();
      }
      void api_t::delete_segment (type::segment_id_t segment_id)
      {
        proto::message_t const reply
          ( communicate ( proto::segment::message_t
                            (proto::segment::del_memory_t (segment_id))
                        )
          );

        std::string const error
          (boost::get<proto::error::error_t> (reply).detail);
        if (!error.empty())
        {
          throw std::runtime_error (error);
        }
      }

      namespace
      {
        gpi::pc::type::handle_id_t
        we_global_range_handle_name_to_handle (we::global::handle const& handle)
        {
          return std::stoul (handle.name(), nullptr, 16);
        }
      }

      std::function<double (std::string const&)>
      api_t::transfer_costs (std::list<std::pair<we::local::range, we::global::range>> const& transfers)
      {
        std::list<gpi::pc::type::memory_region_t> regions;
        for (we::global::range const& range : transfers | boost::adaptors::map_values)
        {
          regions.emplace_back
            ( gpi::pc::type::memory_location_t ( we_global_range_handle_name_to_handle (range.handle())
                                               , range.offset()
                                               )
            , range.size()
            );
        }

        const std::map<std::string, double> costs (transfer_costs (regions));
        return [costs] (std::string const& host) -> double
        {
          return costs.at (host);
        };
      }

      std::map<std::string, double>
      api_t::transfer_costs (std::list<gpi::pc::type::memory_region_t> const& transfers)
      {
        proto::message_t const reply
          (communicate
            ( proto::memory::message_t
              ( proto::memory::get_transfer_costs_t (transfers)
              )
            )
          );

        try
        {
          proto::memory::transfer_costs_t const costs_message
            (boost::get<proto::memory::transfer_costs_t> (boost::get<proto::memory::message_t> (reply)));
          return costs_message.costs;
        }
        catch (boost::bad_get const & ex)
        {
          proto::error::error_t const error (boost::get<proto::error::error_t> (reply));
          throw std::runtime_error (error.detail);
        }
        catch (std::exception const & ex)
        {
          stop ();
          throw;
        }
      }

      gpi::pc::type::handle::descriptor_t
      api_t::info(const gpi::pc::type::handle_t h)
      {
        proto::memory::info_t rqst;
        rqst.handle = h;

        proto::message_t reply (communicate (proto::memory::message_t (rqst)));

        try
        {
          proto::memory::message_t mem_msg (boost::get<proto::memory::message_t>(reply));
          proto::memory::info_reply_t info_rpl (boost::get<proto::memory::info_reply_t>(mem_msg));
          return info_rpl.descriptor;
        }
        catch (boost::bad_get const & ex)
        {
          proto::error::error_t error (boost::get<proto::error::error_t>(reply));
          throw std::runtime_error (error.detail);
        }
        catch (std::exception const & ex)
        {
          stop ();
          throw;
        }
      }

      void *
      api_t::ptr(const gpi::pc::type::handle_t h)
      {
          gpi::pc::type::handle::descriptor_t
            descriptor = info(h);

          lock_type lock(m_mutex);
          segment_map_t::const_iterator seg_it (m_segments.find(descriptor.segment));
          if (seg_it != m_segments.end())
          {
            return seg_it->second->ptr<char>() + descriptor.offset;
          }

          throw std::runtime_error
            ((boost::format("Requested pointer for unknown handle '%1%'") % h
             ).str()
            );
      }

      type::memcpy_id_t api_t::memcpy ( type::memory_location_t const & dst
                                      , type::memory_location_t const & src
                                      , const type::size_t amount
                                      )
      {
        proto::memory::memcpy_t rqst;
        rqst.dst = dst;
        rqst.src = src;
        rqst.size = amount;

        proto::message_t reply (communicate (proto::memory::message_t (rqst)));

        try
        {
          proto::memory::message_t mem_msg (boost::get<proto::memory::message_t>(reply));
          proto::memory::memcpy_reply_t memcpy_rpl (boost::get<proto::memory::memcpy_reply_t>(mem_msg));
          return memcpy_rpl.memcpy_id;
        }
        catch (boost::bad_get const & ex)
        {
          proto::error::error_t error (boost::get<proto::error::error_t>(reply));
          throw std::runtime_error (error.detail);
        }
        catch (std::exception const & ex)
        {
          stop ();
          throw;
        }
      }

      void api_t::wait (type::memcpy_id_t const& memcpy_id)
      {
        proto::memory::wait_t rqst;
        rqst.memcpy_id = memcpy_id;

        proto::message_t reply(communicate (proto::memory::message_t (rqst)));

        try
        {
          proto::memory::message_t mem_msg (boost::get<proto::memory::message_t>(reply));
          proto::memory::wait_reply_t w_rpl (boost::get<proto::memory::wait_reply_t>(mem_msg));
          w_rpl.get();
        }
        catch (boost::bad_get const & ex)
        {
          proto::error::error_t error (boost::get<proto::error::error_t>(reply));
          throw std::runtime_error (error.detail);
        }
        catch (std::exception const & ex)
        {
          stop ();
          throw;
        }
      }

      gpi::pc::type::segment_id_t
      api_t::register_segment( std::string const & name
                             , const gpi::pc::type::size_t sz
                             )
      {
        segment_ptr seg (new gpi::pc::segment::segment_t(name, sz));

        try
        {
          seg->unlink();
        }
        catch (boost::system::system_error const &se)
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

        // communication part
        {
          proto::segment::register_t rqst;
          rqst.name = name;
          rqst.size = sz;

          proto::message_t rply (communicate (proto::segment::message_t (rqst)));
          try
          {
            proto::segment::message_t seg_msg (boost::get<proto::segment::message_t>(rply));
            proto::segment::register_reply_t reg (boost::get<proto::segment::register_reply_t> (seg_msg));
            seg->assign_id (reg.id);
          }
          catch (boost::bad_get const &)
          {
            proto::error::error_t result (boost::get<proto::error::error_t>(rply));
            throw std::runtime_error ("memory segment registration failed: " + result.detail);
          }
          catch (std::exception const & ex)
          {
            stop ();
            throw;
          }
        }

        // critical part begins

        lock_type lock (m_mutex);

        // TODO:
        //   what if we already have a segment with that id?
        //      maybe because the connection went down
        //      how and when is it safe to remove the segment?
        //      client code might be using the data in some way

        if (m_segments.find (seg->id()) != m_segments.end())
        {
          LLOG(WARN, _logger, "There is already a segment attached with id " << seg->id());
          LLOG(WARN, _logger, "DANGER, this looks like an inconsistency!");
          LLOG(WARN, _logger, "moving my one into the trash");

          m_segments.erase(seg->id());
        }

        fhg_assert (m_segments.find (seg->id()) == m_segments.end());

        m_segments [seg->id()] = seg;

        seg->unlink();

        return seg->id();
      }

      void
      api_t::unregister_segment (const gpi::pc::type::segment_id_t id)
      {
        proto::segment::unregister_t rqst;
        rqst.id = id;

        try
        {
          proto::message_t rply (communicate(proto::segment::message_t(rqst)));
          proto::error::error_t result
            (boost::get<proto::error::error_t>(rply));
          if (result.code != proto::error::success)
          {
            LLOG( ERROR
                , _logger
               , "could not unregister segment " << id << ": "
               << result.code << ": " << result.detail
               );
          }
        }
        catch (std::exception const & ex)
        {
          LLOG(ERROR, _logger, "unregister failed: " << ex.what());
        }

        // remove local
        lock_type lock (m_mutex);
        m_segments.erase (id);
      }

      decltype (remote_segment::gaspi) remote_segment::gaspi = {};
      decltype (remote_segment::filesystem) remote_segment::filesystem = {};

      remote_segment::remote_segment
          ( api_t& api
          , decltype (gaspi)
          , type::size_t size
          , type::size_t communication_buffer_size
          , type::size_t num_communication_buffers
          )
        : remote_segment
            ( api
            , "gaspi://?total_size=" + std::to_string (size)
            + "&buffer_size=" + std::to_string (communication_buffer_size)
            + "&buffers=" + std::to_string (num_communication_buffers)
            )
      {}
      remote_segment::remote_segment ( api_t& api
                                     , decltype (filesystem)
                                     , type::size_t size
                                     , boost::filesystem::path location
                                     )
        : remote_segment
            ( api
            , "beegfs://" + location.string()
            + "?total_size=" + std::to_string (size)
            + "&create=true"
            )
      {}

      remote_segment::remote_segment (api_t& api, std::string const& info)
        : _api (api)
        , _segment_id (_api.create_segment (info))
      {}

      remote_segment::~remote_segment()
      {
        _api.delete_segment (_segment_id);
      }
    }
  }
}
