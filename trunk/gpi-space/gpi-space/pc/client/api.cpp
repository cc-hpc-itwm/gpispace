#include "api.hpp"

#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <boost/bind.hpp>

#include <gpi-space/signal_handler.hpp>

#include <fhglog/minimal.hpp>

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
        m_signal_connection =
          gpi::signal::handler().connect
          ( SIGPIPE, boost::bind( &api_t::connection_lost
                                , this
                                , _1
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
    }
  }
}
