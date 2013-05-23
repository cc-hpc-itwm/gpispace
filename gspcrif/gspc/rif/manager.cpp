#include "manager.hpp"

#include <errno.h>
#include <poll.h>               // poll()
#include <fcntl.h> // O_NONBLOCK

#include <boost/foreach.hpp>
#include <boost/system/system_error.hpp>

namespace gspc
{
  namespace rif
  {
    namespace io_thread_command
    {
      enum io_thread_cmd_e
        {
          SHUTDOWN
        };
    }

    namespace detail
    {
      static void add_pollfd ( int fd
                             , int flags
                             , std::vector<pollfd> & to_poll
                             )
      {
        struct pollfd p;
        p.fd = fd;
        p.events = flags;
        p.revents = 0;
        to_poll.push_back (p);
      }
    }

    manager_t::manager_t ()
      : m_mutex ()
      , m_proc_ids ()
      , m_available_proc_ids ()
      , m_io_thread ()
      , m_io_thread_pipe ()
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

      rc = m_io_thread_pipe.open (O_NONBLOCK, true);
      if (rc < 0)
      {
        using namespace boost::system;
        throw system_error (errc::make_error_code ((errc::errc_t)errno));
      }

      m_io_thread.reset
        (new boost::thread ( &manager_t::io_thread
                           , this
                           , boost::ref (m_io_thread_pipe)
                           ));
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

      notify_io_thread (io_thread_command::SHUTDOWN);

      m_io_thread->join ();
      m_io_thread.reset ();
      m_io_thread_pipe.close ();

      {
        unique_lock lock (m_mutex);
        m_stopping = false;
      }
    }

    void manager_t::notify_io_thread (int cmd) const
    {
      unique_lock lock (m_mutex);
      m_io_thread_pipe.write (&cmd, sizeof(cmd));
    }

    void manager_t::io_thread (pipe_t & me)
    {
      bool done = false;

      for (; not done ;)
      {
        int nready;

        // build poll array
        std::vector<pollfd> to_poll;
        detail::add_pollfd (me.rd (), POLLIN, to_poll);

        nready = poll (&to_poll [0], to_poll.size (), -1);

        if (nready < 0)
        {
          if (errno == EINTR)
            continue;
          using namespace boost::system;
          throw system_error (errc::make_error_code ((errc::errc_t)errno));
        }

        if (nready == 0)
        {
          continue;
        }

        BOOST_FOREACH (const pollfd &pfd, to_poll)
        {
          if (pfd.fd == me.rd () && pfd.revents & POLLIN)
          {
            int cmd = -1;
            me.read (&cmd, sizeof(cmd));
            switch (cmd)
            {
            case io_thread_command::SHUTDOWN:
              done = true;
              break;
            default:
              break;
            }

            continue;
          }

          if (pfd.revents & POLLIN)
          {
            // lookup related process
            // read into process buffer
            //    if buffer full, do not poll for read
          }

          if (pfd.revents & POLLOUT)
          {
            // lookup related process
            // write pending data to process
            //    if buffer empty, do not poll for write anymore
          }

          if (pfd.revents & POLLERR || pfd.revents & POLLNVAL)
          {
            // lookup related process and see what can be done about it
            //    close the other side of the pipe
          }

          if (pfd.revents & POLLHUP)
          {
            // lookup related process and see what can be done about it
            //    close the other side of the pipe
          }
        }
      }
    }
  }
}
