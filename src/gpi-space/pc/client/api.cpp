#include <gpi-space/pc/client/api.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/type/flags.hpp>

#include <fhg/assert.hpp>
#include <fhglog/LogMacros.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <list>
#include <utility>

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

        if (m_socket != -1)
        {
          if (ping ())
            return;
          else
            stop ();
        }

        m_socket = open_socket (m_path);

            m_info = boost::get<proto::control::info_reply_t>
              ( boost::get<proto::control::message_t>
               ( communicate
                 (proto::control::message_t (proto::control::info_t()))
               )
              ).info;
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

      bool api_t::is_connected () const
      {
        lock_type lock (m_mutex);
        return m_socket != -1;
      }

      ssize_t api_t::write (const void * buf, size_t sz)
      {
        return fhg::syscall::write (m_socket, buf, sz);
      }

      ssize_t api_t::read (void * buf, size_t sz)
      {
        return fhg::syscall::read (m_socket, buf, sz);
      }

      void api_t::path (std::string const & p)
      {
        m_path = p;
      }
      std::string const & api_t::path () const
      {
        return m_path;
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

      gpi::pc::type::handle_t
      api_t::memset (const gpi::pc::type::handle_t, int, size_t)
      {
        throw std::runtime_error("memset: not yet implemented");
      }

      std::function<double (std::string const&)>
      api_t::transfer_costs (std::list<std::pair<we::local::range, we::global::range>> const&)
      {
        //! \todo get actual values from vmem backend
        return [] (std::string const&) -> double
        {
          return 0.0;
        };
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

      std::vector<gpi::pc::type::size_t>
      api_t::wait ()
      {
        std::vector<gpi::pc::type::size_t> res;
        gpi::pc::type::info::descriptor_t info(collect_info ());
        for (gpi::pc::type::size_t q(0); q < info.queues; ++q)
        {
          res.push_back(wait(q));
        }
        return res;
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

      gpi::pc::type::segment::list_t
      api_t::list_segments ()
      {
        proto::segment::list_t rqst;

        proto::message_t rply (communicate(proto::segment::message_t (rqst)));
        try
        {
          proto::segment::message_t seg_msg (boost::get<proto::segment::message_t>(rply));
          proto::segment::list_reply_t segments (boost::get<proto::segment::list_reply_t> (seg_msg));
          return segments.list;
        }
        catch (boost::bad_get const &)
        {
          proto::error::error_t result
            (boost::get<proto::error::error_t>(rply));
          throw std::runtime_error ("segment listing failed: " + result.detail);
        }
        catch (std::exception const & ex)
        {
          stop ();
          throw;
        }
      }

      void
      api_t::attach_segment (const gpi::pc::type::segment_id_t id)
      {
        {
          lock_type lock (m_mutex);
          if (m_segments.find (id) != m_segments.end())
          {
            throw std::runtime_error ("already attached");
          }
        }

        // get listing of segments
        gpi::pc::type::segment::list_t descriptors
          (list_segments ());

        // find the correct descriptor
        const gpi::pc::type::segment::descriptor_t * desc (nullptr);
        for ( gpi::pc::type::segment::list_t::const_iterator it (descriptors.begin())
            ; it != descriptors.end()
            ; ++it
            )
        {
          if (it->id == id)
          {
            desc = &(*it);
            break;
          }
        }

        if (!desc)
        {
          throw std::runtime_error ("no such segment");
        }

        // open segment
        segment_ptr seg (new gpi::pc::segment::segment_t(desc->name, desc->local_size, id));
          seg->open();

        // communicate
        {
          proto::segment::attach_t rqst;
          rqst.id = id;

          proto::message_t rply (communicate(proto::segment::message_t (rqst)));
          proto::error::error_t result
            (boost::get<proto::error::error_t>(rply));
          if (result.code != proto::error::success)
          {
            throw std::runtime_error ("could not attach to segment: " + result.detail);
          }
          else
          {
            lock_type lock (m_mutex);
            m_segments [seg->id()] = seg;
          }
        }
      }

      void
      api_t::detach_segment (const gpi::pc::type::segment_id_t id)
      {
        proto::segment::detach_t rqst;
        rqst.id = id;

        try
        {
          proto::message_t rply (communicate(proto::segment::message_t(rqst)));
          proto::error::error_t result
            (boost::get<proto::error::error_t>(rply));
          if (result.code != proto::error::success)
          {
            LOG(ERROR, "could not detach from segment: " << result.code << ": " << result.detail);
            // throw or silently ignore?
          }
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "detach failed: " << ex.what());
        }

        lock_type lock (m_mutex);
        m_segments.erase (id);
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

      gpi::pc::type::handle::list_t
      api_t::list_allocations (const gpi::pc::type::segment_id_t seg)
      {
        proto::memory::list_t rqst;
        rqst.segment = seg;

        proto::message_t rply;
        try
        {
          rply = communicate(proto::memory::message_t(rqst));
        }
        catch (std::exception const & ex)
        {
          stop ();
          throw;
        }

        try
        {
          proto::memory::message_t mem_msg (boost::get<proto::memory::message_t> (rply));
          proto::memory::list_reply_t handles (boost::get<proto::memory::list_reply_t>(mem_msg));
          return handles.list;
        }
        catch (boost::bad_get const &)
        {
          proto::error::error_t result (boost::get<proto::error::error_t>(rply));
          LOG(ERROR, "could not get handle list: " << result.code << ": " << result.detail);
          throw std::runtime_error ("handle listing failed: " + result.detail);
        }
        catch (std::exception const & ex)
        {
          stop ();
          throw;
        }
      }

      bool api_t::ping ()
      {
        if (!is_connected())
          return false;

        try
        {
          communicate ( gpi::pc::proto::control::message_t
                        (gpi::pc::proto::control::ping_t())
                      );
          return true;
        }
        catch (std::exception const & ex)
        {
          stop ();
          return false;
        }
      }

      bool api_t::is_attached (const gpi::pc::type::segment_id_t seg_id)
      {
        lock_type lock (m_mutex);
        return m_segments.find (seg_id) != m_segments.end();
      }

      api_t::segment_map_t const & api_t::segments () const
      {
        return m_segments;
      }

      api_t::segment_map_t & api_t::segments ()
      {
        return m_segments;
      }

      api_t::segment_set_t const & api_t::garbage_segments () const
      {
        return m_garbage_segments;
      }

      void api_t::garbage_collect ()
      {
        lock_type lock (m_mutex);
        m_garbage_segments.clear();
      }

      gpi::pc::type::segment_id_t api_t::add_memory (const std::string & url)
      {
        gpi::pc::proto::message_t const rply;
          (communicate (gpi::pc::proto::segment::message_t
                        (gpi::pc::proto::segment::add_memory_t (url)))
          );
        try
        {
          return boost::get<gpi::pc::proto::segment::register_reply_t>
            (boost::get<gpi::pc::proto::segment::message_t>(rply)).id;
        }
        catch (boost::bad_get const &)
        {
          throw
            std::runtime_error (boost::get<proto::error::error_t>(rply).detail);
        }
      }

      void api_t::del_memory (gpi::pc::type::segment_id_t id)
      {
        proto::error::error_t const result
          ( boost::get<proto::error::error_t>
            ( communicate
              ( gpi::pc::proto::segment::message_t
                (proto::segment::del_memory_t (id))
              )
            )
          );

        if (result.code != proto::error::success)
        {
          throw std::runtime_error (result.detail);
        }
      }

      gpi::pc::type::info::descriptor_t api_t::collect_info () const
      {
        return m_info;
      }
    }
  }
}
