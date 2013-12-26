#include "session.hpp"

#include <errno.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <time.h>

#include <gspc/net/handle.hpp>
#include <gspc/net/io.hpp>
#include <gspc/net/serve.hpp>
#include <gspc/net/server.hpp>
#include <gspc/net/service/strip_prefix.hpp>
#include <gspc/kvs/kvs.hpp>
#include <gspc/kvs/impl/kvs_net_service.hpp>
#include <gspc/ctl/system.hpp>

#include <boost/scoped_ptr.hpp>

#include <iostream>

namespace fs = boost::filesystem;

namespace gspc
{
  namespace ctl
  {
    namespace
    {
      bool is_valid_session_name (std::string const &name)
      {
        std::string::const_iterator it = name.begin ();
        const std::string::const_iterator end = name.end ();

        if (it == end)
          return false;

        while (it != end)
        {
          switch (*it)
          {
          case '/':
          case ' ':
            return false;
          default:
            break;
          }

          ++it;
        }

        return true;
      }

      int s_daemonize ()
      {
        if (pid_t child = fork())
        {
          return child;
        }

        setsid ();
        chdir ("/");
        umask (0);

        if (pid_t pid = fork ())
        {
          if (pid > 0)
          {
            exit (0);
          }
          else
          {
            exit (EX_SOFTWARE);
          }
        }

        close(0);
        close(1);
        close(2);

        if (open("/dev/null", O_RDONLY) < 0)
        {
          syslog (LOG_ERR | LOG_USER, "unable to open stdin: /dev/null: %m");
          exit (EX_IOERR);
        }
        if (open ("/dev/null", O_WRONLY) < 0)
        {
          syslog (LOG_ERR | LOG_USER, "unable to open stdout: /dev/null: %m");
          exit (EX_IOERR);
        }
        if (dup (1) < 0)
        {
          syslog (LOG_ERR | LOG_USER, "unable to duplicate stdout->stderr: %m");
          exit (EX_IOERR);
        }

        return 0;
      }
    }

    int session_t::list (fs::path const &dir, session_info_list_t &sessions)
    {
      return session_t::list (dir, sessions, SESSION_ACTIVE);
    }

    int session_t::list ( fs::path const &dir
                        , session_info_list_t & sessions
                        , int flags
                        )
    {
      if (not fs::is_directory (dir))
      {
        return -ENOTDIR;
      }

      bool found_active = false;
      fs::directory_iterator it (dir);
      const fs::directory_iterator end;

      while (it != end)
      {
        gspc::ctl::session_info_t session_info;
        session_info.path = it->path ().string ();

        int rc = session_t::info (it->path (), session_info);
        if (0 == rc || (flags & SESSION_ALL))
        {
          sessions.push_back (session_info);
        }

        found_active = found_active || (0 == rc);

        ++it;
      }

      return found_active ? 0 : -ENOENT;
    }

    int session_t::info (fs::path const &p, session_info_t &info)
    {
      if (not fs::exists (p))
        return -ENOENT;

      try
      {
        int rc;
        gspc::kvs::api_t::value_type val;

        boost::scoped_ptr<gspc::kvs::api_t> api
          (gspc::kvs::create ("unix://" + p.string ()));

        rc = api->get ("gspc.kvs.url", info.puburl);
        if (rc != 0) return rc;

        rc = api->get ("gspc.kvs.pid", info.pid);
        if (rc != 0) return rc;

        rc = api->get ("gspc.kvs.started", info.started);
        if (rc != 0) return rc;

        info.name = p.filename ().string ();
      }
      catch (std::exception const &)
      {
        return -EREMOTEIO;
      }

      return 0;
    }

    session_t::session_t ()
      : m_dir (gspc::ctl::session_directory ())
      , m_name ("default")
      , m_url ("tcp://*:*")
    {}

    int session_t::list (session_info_list_t &sessions) const
    {
      return session_t::list (m_dir, sessions);
    }

    int session_t::list (session_info_list_t &sessions, int flags) const
    {
      return session_t::list (m_dir, sessions, flags);
    }

    int session_t::set_session_dir (fs::path const &p)
    {
      m_dir = p;
      return 0;
    }

    int session_t::get_session_dir (fs::path &p) const
    {
      p = m_dir;
      return 0;
    }

    int session_t::set_session_name (std::string const &name)
    {
      if (not is_valid_session_name (name))
      {
        return -EINVAL;
      }
      m_name = name;
      return 0;
    }

    int session_t::get_session_name (std::string &name) const
    {
      name = m_name;
      return 0;
    }

    int session_t::set_bind_url (std::string const &u)
    {
      m_url = u;
      return 0;
    }

    int session_t::get_bind_url (std::string &u) const
    {
      u = m_url;
      return 0;
    }

    static void s_ignore_signal (int) {}

    int session_t::run (session_info_t & si) const
    {
      if (mkdir (m_dir.c_str (), 0700) < 0)
      {
        if (errno != EEXIST)
        {
          return -errno;
        }

        if (0 == info (m_dir / m_name, si))
        {
          return -EEXIST;
        }
      }

      gspc::net::initializer _net_init;

      gspc::kvs::service_t service;
      gspc::net::handle
        ( "/service/kvs"
        , gspc::net::service::strip_prefix ( "/service/kvs/"
                                           , boost::ref (service)
                                           )
        );

      std::list<gspc::net::server_ptr_t> servers;
      {
        gspc::net::server_ptr_t s (gspc::net::serve (m_url));
        servers.push_back (s);
        si.puburl = s->url ();
      }
      si.started = time (NULL);
      si.pid = getpid ();
      si.name = m_name;
      si.path = (m_dir / m_name).string ();

      service.api ().put ("gspc.kvs.url", si.puburl);
      service.api ().put ("gspc.kvs.pid", si.pid);
      service.api ().put ("gspc.kvs.started", si.started);

      {
        gspc::net::server_ptr_t s
          (gspc::net::serve ("unix://" + (m_dir / m_name).string ()));
        servers.push_back (s);
      }

      if (servers.empty ())
      {
        return -EADDRNOTAVAIL;
      }

      if (isatty (STDOUT_FILENO))
      {
        std::cout << "gspcd: session '" << m_name
                  << "' started with pid: " << getpid ()
                  << " url: " << si.puburl
                  << std::endl;
      }
      else
      {
        syslog ( LOG_USER
               , "session '%s' started with pid: %d"
               , m_name.c_str ()
               , getpid ()
               );
      }

      signal (SIGTERM, s_ignore_signal);
      signal (SIGINT, s_ignore_signal);

      pause ();

      {
        std::list<gspc::net::server_ptr_t>::const_iterator server =
          servers.begin ();
        const std::list<gspc::net::server_ptr_t>::const_iterator end =
          servers.end ();

        while (server != end)
        {
          (*server)->stop ();
          ++server;
        }
      }

      return 0;
    }

    int session_t::daemonize_then_run (session_info_t &si) const
    {
      if (0 == info (m_dir / m_name, si))
      {
        return -EEXIST;
      }

      gspc::net::shutdown ();

      int rc = s_daemonize ();
      if (rc < 0)
        return rc;

      if (0 == rc)
      {
        rc = run (si);
      }
      else
      {
        static const std::size_t CHECK_SOCKET_INTERVAL = 50000;
        static const std::size_t MAX_SOCKET_CHECKS = 10;

        std::size_t timeout
          (CHECK_SOCKET_INTERVAL * MAX_SOCKET_CHECKS);

        fs::path child_socket (m_dir / m_name);
        rc = info (child_socket, si);

        while ((rc == -ENOENT || rc == -ECONNREFUSED) && timeout > 0)
        {
          usleep (CHECK_SOCKET_INTERVAL);
          rc = info (child_socket, si);
          if (0 == rc)
            break;
          timeout -= CHECK_SOCKET_INTERVAL;
        }
      }

      return rc;
    }
  }
}
