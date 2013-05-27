#include "pipe.hpp"

#include <unistd.h>
#include <fcntl.h>

#include <boost/system/system_error.hpp>

namespace gspc
{
  namespace rif
  {
    pipe_t::pipe_t ()
    {
      m_fd [0] = m_fd [1] = -1;
    }

    pipe_t::~pipe_t ()
    {
      this->close ();
    }

    ssize_t pipe_t::read (void *buffer, size_t len)
    {
      return ::read (rd (), buffer, len);
    }

    ssize_t pipe_t::write (const void *buffer, size_t len)
    {
      return ::write (wr (), buffer, len);
    }

    int pipe_t::open (int flags, bool close_on_exec)
    {
      return this->open (flags, flags, close_on_exec);
    }

    int pipe_t::open (int rd_flags, int wr_flags, bool close_on_exec)
    {
      int rc;

      rc = ::pipe (&m_fd[0]);
      if (rc < 0)
      {
        return rc;
      }

      rc = ::fcntl (rd (), F_SETFL, rd_flags);
      if (rc < 0)
      {
        int err = errno;
        close ();
        errno = err;
        return rc;
      }
      rc = ::fcntl (wr (), F_SETFL, wr_flags);
      if (rc < 0)
      {
        int err = errno;
        close ();
        errno = err;
        return rc;
      }

      if (close_on_exec)
      {
        for (size_t i = 0 ; i < 2 ; ++i)
        {
          rc = ::fcntl (m_fd [i], F_SETFD, FD_CLOEXEC);
          if (rc < 0)
          {
            int err = errno;
            close ();
            errno = err;
            return rc;
          }
        }
      }

      return 0;
    }

    int pipe_t::close ()
    {
      int rc = 0;
      rc = close_rd () < 0 ? -1 : rc;
      rc = close_wr () < 0 ? -1 : rc;
      return rc;
    }

    int pipe_t::close_rd ()
    {
      int rc = 0;
      if (m_fd [0] >= 0)
      {
        rc = ::close (m_fd [0]);
        m_fd [0] = -1;
      }
      return rc;
    }

    int pipe_t::close_wr ()
    {
      int rc = 0;
      if (m_fd [1] >= 0)
      {
        rc = ::close (m_fd [1]);
        m_fd [1] = -1;
      }
      return rc;
    }
  }
}
