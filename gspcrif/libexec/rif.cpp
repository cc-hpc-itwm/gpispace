#include "rif.hpp"

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <unistd.h> // gethostname
#include <algorithm>
#include <cctype>

#include <boost/foreach.hpp>

#include <signal.h>

#include <gspc/net.hpp>
#include <gspc/rif.hpp>

class RifImpl;
static RifImpl *s_rif = 0;

namespace detail
{
  static std::string get_hostname ()
  {
    char buf [4096];
    ::gethostname (buf, sizeof(buf));
    return std::string (buf);
  }
}

static std::string const &s_gethostname ()
{
  static std::string h (detail::get_hostname ());
  return h;
}

static void s_handle_rif ( std::string const &dst
                         , gspc::net::frame const &rqst
                         , gspc::net::user_ptr user
                         );

class RifImpl : FHG_PLUGIN
{
public:
  RifImpl ()
  {
    s_rif = this;
  }

  ~RifImpl ()
  {
    s_rif = 0;
  }

  gspc::rif::manager_t & mgr () { return m_mgr; }

  void shutdown ()
  {
    fhg_kernel ()->shutdown ();
  }

  FHG_PLUGIN_START()
  {
    signal (SIGCHLD, SIG_DFL);

    m_mgr.start ();

    gspc::net::handle
      ("/service/rif", &s_handle_rif);
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    m_mgr.stop ();

    FHG_PLUGIN_STOPPED();
  }

  gspc::rif::manager_t m_mgr;
};

void s_handle_rif ( std::string const &dst
                  , gspc::net::frame const &rqst
                  , gspc::net::user_ptr user
                  )
{
  std::string cmd = rqst.get_body_as_string ();

  std::vector<std::string> argv;
  size_t consumed;
  gspc::rif::parse (cmd, argv, consumed);

  if (consumed != cmd.size ())
  {
    user->deliver (gspc::net::make::error_frame ( rqst
                                                , gspc::net::E_SERVICE_FAILED
                                                , "failed to parse command line"
                                                )
                  );
    return;
  }


  std::string command;
  if (not argv.empty ())
  {
    command = argv [0];
    argv.erase (argv.begin ());
  }

  std::transform ( command.begin ()
                 , command.end ()
                 , command.begin ()
                 , ::tolower
                 );

  gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

  if (command == "exec")
  {
    gspc::rif::proc_t p = s_rif->mgr ().exec (argv);

    if (p < 0)
    {
      std::stringstream sstr;
      sstr << "failed: " << -p << ": " << strerror (-p);
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , sstr.str ()
                                          );
      user->deliver (rply);
      return;
    }

    std::stringstream sstr;
    sstr << p << std::endl;
    rply.add_body (sstr.str ());
    user->deliver (rply);
    return;
  }
  else if (command == "status")
  {
    gspc::rif::proc_list_t procs;
    if (argv.empty ())
    {
      procs = s_rif->mgr ().processes ();
    }
    else
    {
      for (size_t i = 0 ; i < argv.size () ; ++i)
      {
        try
        {
          procs.push_back (boost::lexical_cast<gspc::rif::proc_t> (argv [i]));
        }
        catch (boost::bad_lexical_cast const&)
        {
          continue;
        }
      }
    }

    BOOST_FOREACH (gspc::rif::proc_t p, procs)
    {
      std::stringstream sstr;
      sstr << "[" << p << "] ";

      int status;
      int rc = s_rif->mgr ().wait (p, &status, boost::posix_time::seconds (0));
      if (-ESRCH == rc)
      {
        sstr << "no such process";
      }
      else if (rc < 0)
      {
        sstr << "Running";
      }
      else
      {
        sstr << "Terminated: " <<  gspc::rif::make_exit_code (status);
      }

      sstr << std::endl;
      rply.add_body (sstr.str ());
    }
  }
  else if (command == "term")
  {
    gspc::rif::proc_list_t procs;
    if (argv.empty ())
    {
      procs = s_rif->mgr ().processes ();
    }
    else
    {
      for (size_t i = 0 ; i < argv.size () ; ++i)
      {
        try
        {
          procs.push_back (boost::lexical_cast<gspc::rif::proc_t> (argv [i]));
        }
        catch (boost::bad_lexical_cast const&)
        {
          continue;
        }
      }
    }

    BOOST_FOREACH (gspc::rif::proc_t p, procs)
    {
      int rc = s_rif->mgr ().term (p, boost::posix_time::seconds (1));
      if (rc < 0)
      {
        std::stringstream sstr;
        sstr << "failed on: " << p << ": " << strerror (-rc) << std::endl;
        rply.add_body (sstr.str ());
      }
    }
  }
  else if (command == "kill")
  {
    if (argv.size () < 1)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "usage: kill SIG [proc...]"
                                          );
      user->deliver (rply);
      return;
    }

    int sig = 0;
    try
    {
      sig = boost::lexical_cast<int> (argv [0]);
    }
    catch (boost::bad_lexical_cast const&)
    {
      const std::string s (argv[0]);

      if      (s == "HUP")  sig = SIGHUP;
      else if (s == "INT")  sig = SIGINT;
      else if (s == "QUIT") sig = SIGQUIT;
      else if (s == "KILL") sig = SIGKILL;
      else if (s == "TERM") sig = SIGTERM;
      else if (s == "USR1") sig = SIGUSR1;
      else if (s == "USR2") sig = SIGUSR2;
      else if (s == "ALRM") sig = SIGALRM;
      else if (s == "SEGV") sig = SIGSEGV;
      else if (s == "STOP") sig = SIGSTOP;
      else if (s == "CONT") sig = SIGCONT;
      else if (s == "URG")  sig = SIGURG;
      else
      {
        std::stringstream sstr;
        sstr << "invalid sig: " << argv [0];
        rply = gspc::net::make::error_frame ( rqst
                                            , gspc::net::E_SERVICE_FAILED
                                            , sstr.str ()
                                            );
        user->deliver (rply);
        return;
      }
    }

    gspc::rif::proc_list_t procs;
    if (argv.size () == 1)
    {
      procs = s_rif->mgr ().processes ();
    }
    else
    {
      for (size_t i = 1 ; i < argv.size () ; ++i)
      {
        try
        {
          procs.push_back (boost::lexical_cast<gspc::rif::proc_t> (argv [i]));
        }
        catch (boost::bad_lexical_cast const&)
        {
          continue;
        }
      }
    }

    BOOST_FOREACH (gspc::rif::proc_t p, procs)
    {
      std::stringstream sstr;

      sstr << "[" << p << "] ";
      int rc = s_rif->mgr ().kill (p, sig);
      if (rc < 0)
      {
        sstr << "failed: " << strerror (-rc) << std::endl;
      }
      else
      {
        sstr << "ok" << std::endl;
      }
      rply.add_body (sstr.str ());
    }
  }
  else if (command == "purge")
  {
    gspc::rif::proc_list_t procs;
    if (argv.empty ())
    {
      procs = s_rif->mgr ().processes ();
    }
    else
    {
      for (size_t i = 0 ; i < argv.size () ; ++i)
      {
        try
        {
          procs.push_back (boost::lexical_cast<gspc::rif::proc_t> (argv [i]));
        }
        catch (boost::bad_lexical_cast const&)
        {
          continue;
        }
      }
    }

    BOOST_FOREACH (gspc::rif::proc_t p, procs)
    {
      s_rif->mgr ().remove (p);
    }
  }
  else if (command == "ps")
  {
    gspc::rif::proc_list_t procs;
    if (argv.empty ())
    {
      procs = s_rif->mgr ().processes ();
    }
    else
    {
      for (size_t i = 0 ; i < argv.size () ; ++i)
      {
        try
        {
          procs.push_back (boost::lexical_cast<gspc::rif::proc_t> (argv [i]));
        }
        catch (boost::bad_lexical_cast const&)
        {
          continue;
        }
      }
    }

    std::stringstream sstr;
    sstr << "PROC\tPID\tSTATUS\tI\tO\tE\tCOMMAND" << std::endl;
    BOOST_FOREACH (gspc::rif::proc_t p, procs)
    {
      gspc::rif::proc_info_t info;
      int rc = s_rif->mgr ().proc_info (p, info);

      if (0 == rc)
      {
        sstr << info.id ()
             << "\t" << info.pid ()
             << "\t" << (info.status () ? gspc::rif::make_exit_code (*info.status ()) : -EBUSY)
             << "\t" << info.inp_pending ()
             << "\t" << info.out_pending ()
             << "\t" << info.err_pending ()
             << "\t";
        bool first = true;
        BOOST_FOREACH (std::string const &arg, info.argv ())
        {
          if (not first)
          {
            sstr << " ";
          }
          else
          {
            first = false;
          }
          sstr << arg;
        }
        sstr << std::endl;
      }
    }
    rply.set_body (sstr.str ());
  }
  else if (command == "read")
  {
    if (argv.size () < 1)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "usage: read FD [proc...]"
                                          );
      user->deliver (rply);
      return;
    }

    int fd;
    try
    {
      fd = boost::lexical_cast<int> (argv [0]);
    }
    catch (boost::bad_lexical_cast const&)
    {
      std::stringstream sstr;
      sstr << "invalid fd: " << argv [0];
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , sstr.str ()
                                          );
      user->deliver (rply);
      return;
    }

    gspc::rif::proc_list_t procs;
    if (argv.size () == 1)
    {
      procs = s_rif->mgr ().processes ();
    }
    else
    {
      for (size_t i = 1 ; i < argv.size () ; ++i)
      {
        try
        {
          procs.push_back (boost::lexical_cast<gspc::rif::proc_t> (argv [i]));
        }
        catch (boost::bad_lexical_cast const&)
        {
          continue;
        }
      }
    }

    boost::system::error_code ec;

    BOOST_FOREACH (gspc::rif::proc_t p, procs)
    {
      std::stringstream sstr;
      size_t maxlen = 2097152;
      while (maxlen > 0)
      {
        char buf [4096];
        int rc = s_rif->mgr ().read ( p
                                    , fd
                                    , buf
                                    , std::min (sizeof(buf), maxlen)
                                    , ec
                                    );
        if (rc > 0)
        {
          buf [rc] = 0;
          sstr << std::string (&buf[0], rc);
          maxlen -= rc;
        }
        else
        {
          break;
        }
      }
      rply.add_body (sstr.str ());
    }
  }
  else if (command == "write")
  {
    if (argv.size () < 2)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "usage: write proc FD [data...]"
                                          );
      user->deliver (rply);
      return;
    }

    gspc::rif::proc_t p;
    try
    {
      p = boost::lexical_cast<gspc::rif::proc_t> (argv [0]);
    }
    catch (boost::bad_lexical_cast const&)
    {
      std::stringstream sstr;
      sstr << "invalid proc: " << argv [0];
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , sstr.str ()
                                          );
      user->deliver (rply);
      return;
    }

    int fd;
    try
    {
      fd = boost::lexical_cast<int> (argv [1]);
    }
    catch (boost::bad_lexical_cast const&)
    {
      std::stringstream sstr;
      sstr << "invalid fd: " << argv [1];
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , sstr.str ()
                                          );
      user->deliver (rply);
      return;
    }

    for (size_t i = 2 ; i < argv.size () ; ++i)
    {
      boost::system::error_code ec;
      s_rif->mgr ().write (p, fd, argv [i].c_str (), argv [i].size (), ec);
      if (i < (argv.size () - 1))
        s_rif->mgr ().write (p, fd, " ", 1, ec);

      if (ec)
      {
        std::stringstream sstr;
        sstr << "invalid proc: " << argv [0];
        rply = gspc::net::make::error_frame ( rqst
                                            , gspc::net::E_SERVICE_FAILED
                                            , ec.message ()
                                            );
        user->deliver (rply);
        return;
      }
    }
  }
  else if (command == "setenv")
  {
    if (argv.size () != 2)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "usage: setenv <key> <value>"
                                          );
      user->deliver (rply);
      return;
    }

    s_rif->mgr ().setenv (argv [0], argv [1]);
  }
  else if (command == "getenv")
  {
    if (argv.size () != 1)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "usage: getenv <key>"
                                          );
      user->deliver (rply);
      return;
    }

    std::string val;
    if (0 == s_rif->mgr ().getenv (argv [0], val))
    {
      rply.set_body (val);
    }
  }
  else if (command == "delenv")
  {
    if (argv.size () != 1)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "usage: delenv <key>"
                                          );
      user->deliver (rply);
      return;
    }

    s_rif->mgr ().delenv (argv [0]);
  }
  else if (command == "env")
  {
    gspc::rif::env_t env = s_rif->mgr ().env ();
    BOOST_FOREACH (gspc::rif::env_t::value_type const &e, env)
    {
      rply.add_body (e.first);
      rply.add_body ("=");
      rply.add_body (e.second);
      rply.add_body ("\n");
    }
  }
  else if (command == "info")
  {
    std::stringstream sstr;

    sstr << "GspcRIFD running on " << s_gethostname () << " as " << getpid ()
         << std::endl
      ;

    rply.set_body (sstr.str ());
  }
  else if (command == "shutdown")
  {
    s_rif->shutdown ();
  }
  else
  {
    std::stringstream sstr;
    sstr << "usage: rif command <args...>" << std::endl
         << std::endl
         << " commands" << std::endl
         << "   exec command <args...>" << std::endl
         << "   status [proc...]" << std::endl
         << "   term [proc...]" << std::endl
         << "   kill <signal> [proc...]" << std::endl
         << "   read FD [proc...]" << std::endl
         << "   write proc FD data..." << std::endl
         << "   purge [proc...]" << std::endl
         << "   ps [proc...]" << std::endl
         << "   shutdown" << std::endl
      ;
    rply.add_body (sstr.str ());
  }

  user->deliver (rply);
}

EXPORT_FHG_PLUGIN( rif
                 , RifImpl
                 , "rif"
                 , "provides access to the rif infrastructure"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
