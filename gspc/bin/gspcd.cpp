#include <sysexits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <time.h>

#include <iomanip>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <gspc/ctl.hpp>
#include <gspc/net.hpp>
#include <gspc/net/util.hpp>
#include <gspc/kvs.hpp>

#include <gspc/ctl/session.hpp>
#include <gspc/ctl/session_info.hpp>

#include <list>
#include <string>

static std::string puburl;

static void long_usage (int lvl)
{
  std::cerr
    << "usage: gspcd [options] [--] [commands [args...]]"           << std::endl
    << ""                                                           << std::endl
    << "   -h|--help              print this help"                  << std::endl
    << "   -v|--verbose           be more verbose"                  << std::endl
    << "   -f|--force             force startup"                    << std::endl
    << "   --nodaemon             do not daemonize"                 << std::endl
    << ""                                                           << std::endl
    << "   --url URL              were to start the gspc daemon"    << std::endl
    << "                          default: " << puburl              << std::endl
    << ""                                                           << std::endl
    << "   --session NAME         set the session name to use"      << std::endl
    << "   --session-dir DIR      path to the directory where"      << std::endl
    << "                          sessions are located: " << gspc::ctl::session_directory () << std::endl
    << ""                                                           << std::endl
    << "available commands"                                         << std::endl
    << ""                                                           << std::endl
    << "  start"                                                    << std::endl
    << "  stop"                                                     << std::endl
    << "  list | list-sessions"                                     << std::endl
    << "  status"                                                   << std::endl
    << "  url"                                                      << std::endl
    ;
}

static std::ostream & operator << (std::ostream & os, gspc::ctl::session_info_t const &si)
{
  boost::posix_time::ptime started
    (boost::posix_time::from_time_t (si.started));

  boost::posix_time::time_duration uptime
    (boost::posix_time::from_time_t (time (NULL)) - started);

  std::ios_base::fmtflags flgs (os.flags ());

  os.flags (std::ios_base::left);
  os << std::setw (15)
     << si.name
     << " ["
     << "PID=" << si.pid
     << " URL=" << si.puburl
     << " UPTIME=" << uptime
     << "]"
    ;
  os.flags (flgs);

  return os;
}

static int s_list_sessions (std::string const &dir, int verbose)
{
  int rc;

  gspc::ctl::session_t session;
  rc = session.set_session_dir (dir);
  if (0 != rc)
    return rc;

  gspc::ctl::session_info_list_t sessions;
  rc = session.list (sessions);
  if (0 != rc)
    return rc;

  gspc::ctl::session_info_list_t::const_iterator it = sessions.begin ();
  const  gspc::ctl::session_info_list_t::const_iterator end = sessions.end ();

  while (it != end)
  {
    if (verbose)
    {
      std::cout << *it << std::endl;
    }
    else
    {
      std::cout << it->name << std::endl;
    }
    ++it;
  }

  return 0;
}

static int verbose = 0;

int main (int argc, char *argv[])
{
  int rc = 0;
  int i;
  int help = 0;
  int force = 0;
  bool daemonize = true;

  std::string mode ("start");
  std::string session ("default");
  std::string session_dir (gspc::ctl::session_directory ());
  puburl = "tcp://" + gspc::net::hostname () + ":*";

  i = 1;
  while (i < argc)
  {
    std::string arg (argv [i++]);

    if (arg == "--")
    {
      break;
    }

    if (arg.size () > 1 && arg [0] == '-' && arg [1] != '-')
    {
      const char *flag = arg.data ()+1;
      while (*flag != '\0')
      {
        switch (*flag)
        {
        case 'v':
          ++verbose;
          break;
        case 'h':
          ++help;
          break;
        case 'f':
          ++force;
          break;
        default:
          std::cerr << "gspcd: invalid flag: " << *flag << std::endl;
          return EX_USAGE;
        }
        ++flag;
      }
    }
    else if (arg == "--help")
    {
      ++help;
    }
    else if (arg == "--verbose")
    {
      ++verbose;
    }
    else if (arg == "--force")
    {
      ++force;
    }
    else if (arg == "--url")
    {
      if (i == argc)
      {
        std::cerr << "gspcd: missing argument to --url" << std::endl;
        return EX_USAGE;
      }

      puburl = argv [i++];
    }
    else if (arg == "--session")
    {
      if (i == argc)
      {
        std::cerr << "gspcd: missing argument to --session" << std::endl;
        return EX_USAGE;
      }

      session = argv [i++];
    }
    else if (arg == "--session-dir")
    {
      if (i == argc)
      {
        std::cerr << "gspcd: missing argument to --session-dir" << std::endl;
        return EX_USAGE;
      }

      session_dir = argv [i++];
    }
    else if (arg == "--nodaemon")
    {
      daemonize = false;
    }
    else if (arg == "list" || arg == "list-sessions")
    {
      mode = "list";
    }
    else if (arg == "start")
    {
      mode = "start";
    }
    else if (arg == "stop")
    {
      mode = "stop";
    }
    else if (arg == "status")
    {
      mode = "status";
    }
    else if (arg == "url")
    {
      mode = "url";
    }
    else
    {
      std::cerr << "gspcd: invalid option: " << arg << std::endl;
      return EX_USAGE;
    }
  }

  if (help)
  {
    long_usage (help);
    return 0;
  }

  if (mode == "list")
  {
    int ec = s_list_sessions (session_dir, verbose);
    switch (ec)
    {
    case -EPERM:
      rc = EX_NOPERM;
      break;
    case -ENOENT:
      rc = EX_UNAVAILABLE;
      break;
    case 0:
      rc = 0;
      break;
    default:
      rc = EX_SOFTWARE;
      break;
    }
  }
  else if (mode == "status")
  {
    gspc::ctl::session_info_t info;
    rc = gspc::ctl::session_t::info ( boost::filesystem::path (session_dir) / session
                                    , info
                                    );
    if (0 == rc)
    {
      if (verbose)
      {
        std::cout << "ALIVE: " << info << std::endl;
      }
    }
    else if (rc == -ECONNREFUSED)
    {
      if (verbose)
      {
        std::cout << "DEAD: " << session << std::endl;
      }
    }
    else
    {
      if (verbose)
      {
        std::cout << "ERROR: " << session << ": " << strerror (-rc) << std::endl;
      }
    }
  }
  else if (mode == "stop")
  {
    gspc::ctl::session_info_t info;
    rc = gspc::ctl::session_t::info ( boost::filesystem::path (session_dir) / session
                                    , info
                                    );
    if (0 == rc)
    {
      if (force)
      {
        kill (info.pid, SIGKILL);
        if (verbose)
        {
          std::cerr << "sent SIGKILL to " << info.pid << std::endl;
        }
      }
      else
      {
        kill (info.pid, SIGTERM);
        if (verbose)
        {
          std::cerr << "sent SIGTERM to " << info.pid << std::endl;
        }
      }
    }
    else
    {
      std::cerr << "no such session: " << session << std::endl;
      rc = EX_UNAVAILABLE;
    }
  }
  else if (mode == "url")
  {
    gspc::ctl::session_info_t info;
    rc = gspc::ctl::session_t::info ( boost::filesystem::path (session_dir) / session
                                    , info
                                    );
    if (0 == rc)
    {
      std::cout << info.puburl << std::endl;
    }
    else
    {
      std::cerr << "no such session: " << session << std::endl;
      rc = EX_UNAVAILABLE;
    }
  }
  else if (mode == "start")
  {
    gspc::ctl::session_t s;
    s.set_session_dir (session_dir);
    s.set_session_name (session);
    s.set_bind_url (puburl);

    gspc::ctl::session_info_t info;

    if (daemonize)
    {
      rc = s.daemonize_then_run (info);
    }
    else
    {
      rc = s.run (info);
    }

    if (0 == rc)
    {
      std::cout << info.puburl << std::endl;
    }
    else if (-EEXIST == rc)
    {
      std::cerr << "session '" << session << "' still running: "
                << "[" << info.pid << "]"
                << std::endl
        ;
      rc = EX_TEMPFAIL;
    }
    else
    {
      std::cerr << "failed to start: " << strerror (-rc) << std::endl;
      rc = EX_UNAVAILABLE;
    }
  }
  else
  {
    std::cerr << "gspcd: mode '" << mode << "' not yet implemented" << std::endl;
    return EX_SOFTWARE;
  }

  if (rc < 0)
  {
    rc = EX_SOFTWARE;
  }

  return rc;
}
