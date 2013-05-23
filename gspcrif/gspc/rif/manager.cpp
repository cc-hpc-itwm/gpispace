#include "manager.hpp"

#include <errno.h>
#include <unistd.h>             // pipe
#include <fcntl.h>              /* Obtain O_* constant definitions */

#include <boost/system/system_error.hpp>

namespace gspc
{
  namespace rif
  {
    namespace detail
    {
      int pipe (int fd[2], int flags)
      {
        int rc;

        rc = ::pipe (fd);
        if (rc < 0)
        {
          return rc;
        }

        rc = fcntl (fd [0], F_SETFL, flags);
        if (rc < 0)
        {
          int err = errno;
          close (fd [0]);
          close (fd [1]);
          errno = err;
          return rc;
        }

        rc = fcntl (fd [1], F_SETFL, flags);
        if (rc < 0)
        {
          int err = errno;
          close (fd [0]);
          close (fd [1]);
          errno = err;
          return rc;
        }

        return 0;
      }
    }

    manager_t::manager_t ()
      : m_mutex ()
      , m_proc_ids ()
      , m_available_proc_ids ()
      , m_io_thread ()
    {}

    manager_t::~manager_t ()
    {
      stop ();
    }

    void manager_t::start ()
    {
      int rc;

      unique_lock lock (m_mutex);
      if (m_io_thread)
        return;
      int io_thread_pipe [2];
      rc = detail::pipe (io_thread_pipe, O_NONBLOCK);
      if (rc < 0)
      {
        using namespace boost::system;
        throw system_error (errc::make_error_code ((errc::errc_t)errno));
      }

      m_pipe_to_io_thread = io_thread_pipe [1];
      m_io_thread.reset
        (new boost::thread (&manager_t::io_thread, this, io_thread_pipe [0]));
    }

    void manager_t::stop ()
    {
      {
        unique_lock lock (m_mutex);
        if (not m_io_thread)
        {
          return;
        }

        m_stopping = true;
      }

      // kill all procs

      notify_io_thread ();

      // join io thread
      m_io_thread->join ();

      {
        unique_lock lock (m_mutex);
        m_stopping = false;
      }
    }

    void manager_t::notify_io_thread () const
    {
      unique_lock lock (m_mutex);
      int data = 0;
      write (m_pipe_to_io_thread, &data, sizeof(data));
    }

    void manager_t::io_thread (int fd)
    {
      if (fd < 0)
        return;

      for (;;)
      {
        int cmd = -1;
        read (fd, &cmd, sizeof(cmd));
        if (cmd == 0)
        {
          break;
        }
      }

      close (fd);
    }
  }
}
