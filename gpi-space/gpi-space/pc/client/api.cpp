#include "api.hpp"

#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <fstream>

#include <boost/bind.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <fhglog/minimal.hpp>
#include <fhg/assert.hpp>

#include <gpi-space/pc/proto/message.hpp>

static int close_socket (const int fd)
{
  shutdown (fd, SHUT_RDWR);
  return close (fd);
}

static int open_socket (std::string const & path)
{
  int sfd, err;
  struct sockaddr_un addr;

  sfd = socket (AF_UNIX, SOCK_STREAM, 0);
  if (sfd < 0)
  {
    return -errno;
  }

  memset (&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy (addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

  err = connect (sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un));
  if (err < 0)
  {
    err = -errno;
    close_socket (sfd);
    return err;
  }

  return sfd;
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
        , m_connected (false)
      {}

      api_t::~api_t ()
      {
        try
        {
          stop ();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not stop pc client api: " << ex.what());
        }
      }

      void api_t::start ()
      {
        lock_type lock (m_mutex);

        if (m_connected)
        {
          stop ();
        }

        m_socket = open_socket (m_path);
        if (m_socket < 0)
        {
          throw std::runtime_error (strerror(errno));
        }
        else
        {
          m_connected = true;

          try
          {
            m_info = _collect_info ();
          }
          catch (...)
          {
            stop();
            throw;
          }
        }
      }

      void api_t::stop ()
      {
        lock_type lock (m_mutex);

        if (m_connected)
        {
          close_socket (m_socket);
          m_socket = -1;
          m_connected = false;

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
        return m_connected;
      }

      int api_t::write (const void * buf, size_t sz)
      {
        return ::write (m_socket, buf, sz);
      }

      int api_t::read (void * buf, size_t sz)
      {
        return ::read (m_socket, buf, sz);
      }

      int api_t::connection_lost (int)
      {
        stop();
        return 0;
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

        using namespace gpi::pc::proto;
        int err;

        // serialize
        std::string data;
        try
        {
          std::stringstream sstr;
          boost::archive::binary_oarchive oa (sstr);
          oa & rqst;
          data = sstr.str();
        }
        catch (std::exception const &ex)
        {
          MLOG(ERROR, "could not serialize request: " << ex.what());
          throw;
        }

        // send
        header_t header;
        ::memset (&header, 0, sizeof(header));
        header.version = 0x01;
        header.length = data.size();

        err = this->write (&header, sizeof(header));
        if (err <= 0)
        {
          stop ();
          throw std::runtime_error ("could not send data");
        }

        err = this->write (data.c_str(), data.size());
        if (err <= 0)
        {
          stop ();
          throw std::runtime_error ("could not send data");
        }

        // receive
        std::vector<char> buffer;
        err = this->read  (&header, sizeof (header));
        if (err <= 0)
        {
          stop ();
          throw std::runtime_error ("could not receive data");
        }

        buffer.resize (header.length);
        err = this->read (&buffer[0], header.length);
        if (err <= 0)
        {
          stop ();
          throw std::runtime_error ("could not receive data");
        }

        message_t rply;
        // deserialize
        try
        {
          std::stringstream sstr (std::string(buffer.begin(), buffer.end()));
          boost::archive::text_iarchive ia (sstr);
          ia & rply;
        }
        catch (std::exception const & ex)
        {
          throw std::runtime_error ("protocol missmatch: could not deserialize message");
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
        using namespace gpi::pc;

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
          DLOG(INFO, "allocation successful: " << gpi::pc::type::handle_t(alloc_rpl.handle));
          return alloc_rpl.handle;
        }
        catch (boost::bad_get const & ex)
        {
          proto::error::error_t error (boost::get<proto::error::error_t>(reply));
          DLOG(WARN, "request failed: " << error.code << ": " << error.detail);
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
        using namespace gpi::pc;
        proto::memory::free_t rqst;
        rqst.handle = hdl;

        proto::message_t rpl (communicate (proto::memory::message_t (rqst)));
        proto::error::error_t result (boost::get<proto::error::error_t>(rpl));
        if (result.code != proto::error::success)
        {
          DLOG(WARN, "could not free handle: " << result.code << ": " << result.detail);
          throw std::runtime_error ("handle could not be free'd: " + result.detail);
        }
      }

      gpi::pc::type::handle_t
      api_t::memset (const gpi::pc::type::handle_t h, int value, size_t count)
      {
        throw std::runtime_error("memset: not yet implemented");
      }

      gpi::pc::type::handle::descriptor_t
      api_t::info(const gpi::pc::type::handle_t h)
      {
        using namespace gpi::pc;
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
          DLOG(WARN, "request failed: " << error.code << ": " << error.detail);
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
            return 0;
          }
        }
        catch (std::exception const &)
        {
          return 0;
        }
      }

      gpi::pc::type::queue_id_t
      api_t::memcpy ( gpi::pc::type::memory_location_t const & dst
                    , gpi::pc::type::memory_location_t const & src
                    , const gpi::pc::type::size_t amount
                    , const gpi::pc::type::queue_id_t queue
                    )
      {
        using namespace gpi::pc;
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
          DLOG(INFO, "memcpy in progress using queue " << memcpy_rpl.queue);
          return memcpy_rpl.queue;
        }
        catch (boost::bad_get const & ex)
        {
          proto::error::error_t error (boost::get<proto::error::error_t>(reply));
          DLOG(WARN, "request failed: " << error.code << ": " << error.detail);
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
        using namespace gpi::pc;
        proto::memory::wait_t rqst;
        rqst.queue = queue;

        proto::message_t reply(communicate (proto::memory::message_t (rqst)));

        try
        {
          proto::memory::message_t mem_msg (boost::get<proto::memory::message_t>(reply));
          proto::memory::wait_reply_t w_rpl (boost::get<proto::memory::wait_reply_t>(mem_msg));
          DLOG(INFO, "wait on queue " << queue << " returned " << w_rpl.count);
          return w_rpl.count;
        }
        catch (boost::bad_get const & ex)
        {
          proto::error::error_t error (boost::get<proto::error::error_t>(reply));
          DLOG(WARN, "request failed: " << error.code << ": " << error.detail);
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
        try
        {
          if (flags & gpi::pc::type::segment::F_NOCREATE)
          {
            seg->open();
          }
          else
          {
            if (flags & gpi::pc::type::segment::F_FORCE_UNLINK)
            {
              seg->unlink();
            }
            seg->create ();
          }
        }
        catch (std::exception const & ex)
        {
          throw;
        }

        // communication part
        {
          using namespace gpi::pc;
          proto::segment::register_t rqst;
          rqst.name = name;
          rqst.size = sz;
          rqst.flags = flags | gpi::pc::type::segment::F_NOCREATE;

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
            DLOG(ERROR, "could not register segment: " << result.code << ": " << result.detail);
            //          throw wrap (result);
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

        assert (m_segments.find (seg->id()) == m_segments.end());

        m_segments [seg->id()] = seg;
        return seg->id();
      }

      gpi::pc::type::segment::list_t
      api_t::list_segments ()
      {
        using namespace gpi::pc;
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
          DLOG(ERROR, "could not get segment list: " << result.code << ": " << result.detail);
          //          throw wrap (result);
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
        const gpi::pc::type::segment::descriptor_t * desc (NULL);
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
          DLOG(ERROR, "segment descriptor not found: " << id);
          throw std::runtime_error ("no such segment");
        }

        // open segment
        segment_ptr seg (new gpi::pc::segment::segment_t(desc->name, desc->local_size, id));
        try
        {
          seg->open();
        }
        catch (std::exception const & ex)
        {
          DLOG(ERROR, "could not open segment " << desc->name << ": " << ex.what());
          throw;
        }

        // communicate
        {
          using namespace gpi::pc;
          proto::segment::attach_t rqst;
          rqst.id = id;

          proto::message_t rply (communicate(proto::segment::message_t (rqst)));
          proto::error::error_t result
            (boost::get<proto::error::error_t>(rply));
          if (result.code != proto::error::success)
          {
            DLOG(ERROR, "could not attach to segment: " << result.code << ": " << result.detail);
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
        using namespace gpi::pc;
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
        using namespace gpi::pc;
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
        using namespace gpi::pc;
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

        gpi::pc::proto::control::ping_t p;
        try
        {
          communicate (gpi::pc::proto::control::message_t (p));
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
        while (! m_garbage_segments.empty())
        {
          DLOG(INFO, "garbage collecting segment: " << **m_garbage_segments.begin());
          m_garbage_segments.erase (m_garbage_segments.begin());
        }
      }

      gpi::pc::type::segment_id_t api_t::add_memory (const std::string & url)
      {
        gpi::pc::proto::segment::add_memory_t msg (url);
        gpi::pc::proto::message_t rply;
        try
        {
          rply = communicate (gpi::pc::proto::segment::message_t (msg));
          gpi::pc::proto::segment::message_t result
            (boost::get<gpi::pc::proto::segment::message_t>(rply));
          return
            boost::get<gpi::pc::proto::segment::register_reply_t>(result).id;
        }
        catch (boost::bad_get const &)
        {
          proto::error::error_t result
            (boost::get<proto::error::error_t>(rply));
          throw
            std::runtime_error (result.detail);
        }
        catch (std::exception const & ex)
        {
          throw;
        }
      }

      void api_t::del_memory (gpi::pc::type::segment_id_t id)
      {
        gpi::pc::proto::segment::del_memory_t msg (id);

        gpi::pc::proto::message_t rply
          (communicate (gpi::pc::proto::segment::message_t (msg)));
        proto::error::error_t result
          (boost::get<proto::error::error_t>(rply));
        if (result.code != proto::error::success)
        {
          throw std::runtime_error (result.detail);
        }
      }

      gpi::pc::type::info::descriptor_t api_t::collect_info () const
      {
        return m_info;
      }

      gpi::pc::type::info::descriptor_t api_t::_collect_info ()
      {
        gpi::pc::proto::control::info_t msg;
        try
        {
          gpi::pc::proto::message_t rply
            (communicate (gpi::pc::proto::control::message_t (msg)));
          gpi::pc::proto::control::message_t ctrl_msg
            (boost::get<gpi::pc::proto::control::message_t>(rply));
          return boost::get<gpi::pc::proto::control::info_reply_t>(ctrl_msg).info;
        }
        catch (std::exception const & ex)
        {
          throw;
        }
      }
    }
  }
}
