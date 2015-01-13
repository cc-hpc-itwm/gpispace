#include <gpi-space/pc/client/api.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/type/flags.hpp>

#include <fhg/assert.hpp>
#include <fhglog/LogMacros.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/range/adaptor/map.hpp>

#include <sys/un.h>

namespace
{
void close_socket (const int fd)
{
  fhg::syscall::shutdown (fd, SHUT_RDWR);
  fhg::syscall::close (fd);
}

int open_socket (std::string const & path)
{
  int const sfd = fhg::syscall::socket (AF_UNIX, SOCK_STREAM, 0);

  struct sockaddr_un addr;

  memset (&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy (addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

  try
  {
    fhg::syscall::connect (sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un));
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
      api_t::api_t (std::string const & path)
        : m_path (path)
        , m_socket (-1)
      {}

      api_t::~api_t ()
      {
          stop ();
      }

      void api_t::start ()
      {
        lock_type lock (m_mutex);

        m_socket = open_socket (m_path);
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

          // move all segments to trash
          while (! m_segments.empty())
          {
            m_garbage_segments.insert (m_segments.begin()->second);
            m_segments.erase (m_segments.begin());
          }
        }
      }

      ssize_t api_t::write (const void * buf, size_t sz)
      {
        return fhg::syscall::write (m_socket, buf, sz);
      }

      ssize_t api_t::read (void * buf, size_t sz)
      {
        return fhg::syscall::read (m_socket, buf, sz);
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

        if ( (this->write (&header, sizeof(header)) <= 0)
           || (this->write (data.c_str(), data.size()) <= 0)
           )
        {
          stop ();
          throw std::runtime_error ("could not send data");
        }

        // receive
        if (this->read  (&header, sizeof (header)) <= 0)
        {
          stop ();
          throw std::runtime_error ("could not receive data header");
        }

        std::vector<char> buffer (header.length);
        if (this->read (&buffer[0], header.length) <= 0)
        {
          stop ();
          throw std::runtime_error ("could not receive data");
        }

        proto::message_t rply;
        // deserialize
        {
          std::stringstream sstr (std::string(buffer.begin(), buffer.end()));
          boost::archive::text_iarchive ia (sstr);
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
        try
        {
          gpi::pc::type::handle::descriptor_t
            descriptor = info(h);

          lock_type lock(m_mutex);
          segment_map_t::const_iterator seg_it (m_segments.find(descriptor.segment));
          if (seg_it != m_segments.end())
          {
            return seg_it->second->ptr<char>() + descriptor.offset;
          }
          else
          {
            return nullptr;
          }
        }
        catch (std::exception const &)
        {
          return nullptr;
        }
      }

      gpi::pc::type::queue_id_t
      api_t::memcpy ( gpi::pc::type::memory_location_t const & dst
                    , gpi::pc::type::memory_location_t const & src
                    , const gpi::pc::type::size_t amount
                    , const gpi::pc::type::queue_id_t queue
                    )
      {
        proto::memory::memcpy_t rqst;
        rqst.dst = dst;
        rqst.src = src;
        rqst.size = amount;
        rqst.queue = queue;

        proto::message_t reply (communicate (proto::memory::message_t (rqst)));

        try
        {
          proto::memory::message_t mem_msg (boost::get<proto::memory::message_t>(reply));
          proto::memory::memcpy_reply_t memcpy_rpl (boost::get<proto::memory::memcpy_reply_t>(mem_msg));
          return memcpy_rpl.queue;
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

      gpi::pc::type::size_t
      api_t::wait (const gpi::pc::type::queue_id_t queue)
      {
        proto::memory::wait_t rqst;
        rqst.queue = queue;

        proto::message_t reply(communicate (proto::memory::message_t (rqst)));

        try
        {
          proto::memory::message_t mem_msg (boost::get<proto::memory::message_t>(reply));
          proto::memory::wait_reply_t w_rpl (boost::get<proto::memory::wait_reply_t>(mem_msg));
          return w_rpl.count;
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
                             , const gpi::pc::type::flags_t flags
                             )
      {
        segment_ptr seg (new gpi::pc::segment::segment_t(name, sz));
          if (flags & gpi::pc::F_NOCREATE)
          {
            seg->open();
          }
          else
          {
            if (flags & gpi::pc::F_FORCE_UNLINK)
            {
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
            }
            seg->create ();
          }

        // communication part
        {
          proto::segment::register_t rqst;
          rqst.name = name;
          rqst.size = sz;
          rqst.flags = flags | gpi::pc::F_NOCREATE;

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
          LOG(WARN, "There is already a segment attached with id " << seg->id());
          LOG(WARN, "DANGER, this looks like an inconsistency!");
          LOG(WARN, "    my segment: " << *m_segments.at(seg->id()));
          LOG(WARN, "   new segment: " << *seg);
          LOG(WARN, "moving my one into the trash");

          m_garbage_segments.insert (m_segments.at(seg->id()));
          m_segments.erase(seg->id());
        }

        fhg_assert (m_segments.find (seg->id()) == m_segments.end());

        m_segments [seg->id()] = seg;

        if (gpi::flag::is_set (flags, gpi::pc::F_EXCLUSIVE))
        {
          seg->unlink();
        }

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
            LOG( ERROR
               , "could not unregister segment " << id << ": "
               << result.code << ": " << result.detail
               );
          }
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "unregister failed: " << ex.what());
        }

        // remove local
        lock_type lock (m_mutex);
        m_segments.erase (id);
      }
    }
  }
}
