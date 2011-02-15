#include "api.hpp"

#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <boost/bind.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/signal_handler.hpp>
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
      {
        m_sigpipe_connection =
          gpi::signal::handler().connect
          ( SIGPIPE, boost::bind( &api_t::connection_lost
                                , this
                                , _1
                                )
          );
        m_sigint_connection =
          gpi::signal::handler().connect
          ( SIGINT, boost::bind( &api_t::stop
                               , this
                               )
          );
      }

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
        }
      }

      void api_t::stop ()
      {
        if (m_connected)
        {
          close_socket (m_socket);
          m_socket = -1;
          m_connected = false;
        }
      }

      bool api_t::is_connected () const
      {
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
        using namespace gpi::pc::proto;
        int err;

        // serialize
        std::string data;
        {
          std::stringstream sstr;
          boost::archive::text_oarchive oa (sstr);
          oa & rqst;
          data = sstr.str();
        }

        // send
        header_t header;
        header.version = 0x01;
        header.length = data.size();

        err = this->write (&header, sizeof(header));
        err = this->write (data.c_str(), data.size());

        // receive
        std::vector<char> buffer;
        err = this->read  (&header, sizeof (header));

        buffer.resize (header.length);
        err = this->read (&buffer[0], header.length);

        message_t rply;
        // deserialize
        {
          std::stringstream sstr (std::string(buffer.begin(), buffer.end()));
          boost::archive::text_iarchive ia (sstr);
          ia & rply;
        }
        return rply;
      }

      type::handle_id_t api_t::alloc ( const type::segment_id_t seg
                                     , const type::size_t sz
                                     , const std::string & name
                                     , const type::mode_t flags
                                     )
      {
        using namespace gpi::pc::proto;

        memory::alloc_t alloc_msg;
        alloc_msg.segment = seg;
        alloc_msg.size = sz;
        alloc_msg.name = name;
        alloc_msg.flags = flags;

        message_t reply (communicate (alloc_msg));

        try
        {
          memory::alloc_reply_t alloc_rpl (boost::get<memory::alloc_reply_t>(reply));
          LOG(INFO, "allocation successful: " << alloc_rpl.handle);
          return alloc_rpl.handle;
        }
        catch (boost::bad_get const & ex)
        {
          LOG(ERROR, "request failed: " << boost::get<error::error_t>(reply).code);
          throw std::runtime_error ("could not allocate");
        }
      }

      void api_t::free (const gpi::pc::type::handle_id_t hdl)
      {
        using namespace gpi::pc::proto;
        memory::free_t rqst;
        rqst.handle = hdl;

        message_t rpl (communicate (rqst));
        error::error_t result (boost::get<error::error_t>(rpl));
        if (result.code != error::success)
        {
          LOG(ERROR, "could not free handle: " << result.code << ": " << result.detail);
          // throw or silently ignore?
        }
      }

      gpi::pc::type::segment_id_t
      api_t::register_segment( std::string const & name
                             , const gpi::pc::type::size_t sz
                             , const gpi::pc::type::flags_t flags
                             )
      {
        using namespace gpi::pc::proto;
        segment::register_t rqst;
        rqst.name = name;
        rqst.size = sz;
        rqst.flags = flags;

        message_t rply (communicate (rqst));
        try
        {
          segment::register_reply_t reg (boost::get<segment::register_reply_t> (rply));
          return reg.id;
        }
        catch (boost::bad_get const &)
        {
          error::error_t result (boost::get<error::error_t>(rply));
          LOG(ERROR, "could not register segment: " << result.code << ": " << result.detail);
          //          throw wrap (result);
          throw std::runtime_error ("memory segment registration failed: " + result.detail);
        }
      }

      gpi::pc::type::segment::list_t
      api_t::list_segments ()
      {
        using namespace gpi::pc::proto;
        segment::list_t rqst;

        message_t rply (communicate(rqst));
        try
        {
          segment::list_reply_t segments (boost::get<segment::list_reply_t> (rply));
          return segments.list;
        }
        catch (boost::bad_get const &)
        {
          error::error_t result (boost::get<error::error_t>(rply));
          LOG(ERROR, "could not get segment list: " << result.code << ": " << result.detail);
          //          throw wrap (result);
          throw std::runtime_error ("segment listing failed: " + result.detail);
        }
      }
    }
  }
}
