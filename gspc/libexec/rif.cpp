// alexander.petry@itwm.fraunhofer.de

#include <libexec/rif.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/util/hostname.hpp>
#include <fhg/util/num.hpp>
#include <fhg/util/split.hpp>

#include <algorithm>
#include <cctype>

#include <boost/foreach.hpp>

#include <signal.h>

#include <gspc/net/error.hpp>
#include <gspc/net/frame_builder.hpp>
#include <gspc/net/io.hpp>
#include <gspc/net/serve.hpp>
#include <gspc/net/server.hpp>
#include <gspc/net/server/default_queue_manager.hpp>
#include <gspc/net/server/default_service_demux.hpp>
#include <gspc/net/service/echo.hpp>
#include <gspc/net/user.hpp>
#include <gspc/rif/manager.hpp>
#include <gspc/rif/proc_info.hpp>
#include <gspc/rif/supervisor.hpp>
#include <gspc/rif/util.hpp>

  RifImpl::RifImpl ( boost::function<void()> request_shutdown
                   , size_t nthreads, std::string netd_url
                   )
    : _request_shutdown (request_shutdown)
    , m_server()
    , m_mgr ()
    , m_supervisor (m_mgr)
  {
    gspc::net::initialize (nthreads);

    gspc::net::server::default_service_demux().handle
      ("/service/echo", gspc::net::service::echo ());

    m_server = gspc::net::serve
      (netd_url, gspc::net::server::default_queue_manager());

    MLOG (DEBUG, "listening on " << m_server->url ());

    signal (SIGCHLD, SIG_DFL);

    m_supervisor.onChildFailed.connect
      (boost::bind (&RifImpl::on_child_failed, this, _1));
    m_supervisor.onChildStarted.connect
      (boost::bind (&RifImpl::on_child_started, this, _1));
    m_supervisor.onChildTerminated.connect
      (boost::bind (&RifImpl::on_child_terminated, this, _1));

    m_supervisor.start ();

    gspc::net::server::default_service_demux().handle
      ("/service/rif", boost::bind (&RifImpl::handle, this, _1, _2, _3));
  }

  RifImpl::~RifImpl()
  {
    m_supervisor.onChildFailed.connect
      (boost::bind (&RifImpl::on_child_failed, this, _1));
    m_supervisor.onChildStarted.connect
      (boost::bind (&RifImpl::on_child_started, this, _1));
    m_supervisor.onChildTerminated.connect
      (boost::bind (&RifImpl::on_child_terminated, this, _1));

    m_supervisor.stop ();

    if (m_server)
    {
      m_server->stop ();
    }

    gspc::net::shutdown ();
  }

  int RifImpl::supervise (std::list<std::string> argv)
  {
    gspc::rif::child_descriptor_t child;
    child.name = "";
    child.restart_mode = gspc::rif::child_descriptor_t::RESTART_ALWAYS;
    child.shutdown_mode = gspc::rif::child_descriptor_t::SHUTDOWN_KILL;

    while (argv.size ())
    {
      std::string arg = argv.front ();

      if       (arg == "--")
      {
        argv.erase (argv.begin ());
        break;
      }
      else if (arg.size () > 2 && arg [0] == '-' && arg [1] == '-')
      {
        argv.pop_front ();
        if       (arg == "--name")
        {
          if (argv.empty ())
          {
            throw std::runtime_error ("supervise: --name needs an argument");
          }
          child.name = argv.front ();
          argv.pop_front ();
        }
        else if (arg == "--max-restarts")
        {
          if (argv.empty ())
          {
            throw std::runtime_error ("supervise: --max-restarts needs an argument");
          }

          std::string value = argv.front ();
          argv.pop_front ();

          try
          {
            child.max_restarts = fhg::util::read_size_t (value);
          }
          catch (std::exception const &)
          {
            throw std::runtime_error
              ("supervise: invalid max-restarts: " + value);
          }
        }
        else if (arg == "--max-start-time")
        {
          if (argv.empty ())
          {
            throw std::runtime_error ("supervise: --max-start-time needs an argument");
          }

          std::string value = argv.front ();
          argv.pop_front ();

          try
          {
            child.max_start_time = fhg::util::read_size_t (value);
          }
          catch (std::exception const &)
          {
            throw std::runtime_error
              ("supervise: invalid max-start-time: " + value);
          }
        }
        else if (arg == "--restart")
        {
          if (argv.empty ())
          {
            throw std::runtime_error ("supervise: --mode needs an argument");
          }

          std::string mode = argv.front ();
          argv.pop_front ();

          if (mode == "never")
            child.restart_mode = gspc::rif::child_descriptor_t::RESTART_NEVER;
          else if (mode == "always")
            child.restart_mode = gspc::rif::child_descriptor_t::RESTART_ALWAYS;
          else if (mode == "only_if_failed")
            child.restart_mode = gspc::rif::child_descriptor_t::RESTART_ONLY_IF_FAILED;
          else
          {
            throw std::runtime_error
              ("supervise: invalid restart mode: " + mode);
          }
        }
        else if (arg == "--shutdown")
        {
          if (argv.empty ())
          {
            throw std::runtime_error ("supervise: --mode needs an argument");
          }

          std::string mode = argv.front ();
          argv.pop_front ();

          if (mode == "kill")
            child.shutdown_mode = gspc::rif::child_descriptor_t::SHUTDOWN_KILL;
          else if (mode == "wait")
            child.shutdown_mode = gspc::rif::child_descriptor_t::SHUTDOWN_INFINITY;
          else
          {
            try
            {
              child.timeout = fhg::util::read_int (mode);
              child.shutdown_mode = gspc::rif::child_descriptor_t::SHUTDOWN_WITH_TIMEOUT;
            }
            catch (std::exception const &)
            {
              throw std::runtime_error
                ("supervise: invalid shutdown mode: " + mode);
            }
          }
        }
        else if (arg == "--setenv")
        {
          if (argv.empty ())
          {
            throw std::runtime_error ("supervise: --setenv needs an argument");
          }

          const std::pair<std::string, std::string> env_kvp =
            fhg::util::split_string (argv.front (), "=");
          child.env.insert (env_kvp);
        }
      }
      else
      {
        break;
      }
    }

    if (argv.empty ())
    {
      throw std::runtime_error ("supervise: command is missing");
    }

    if (child.name.empty ())
    {
      throw std::runtime_error ("supervise: --name is required");
    }

    child.argv.assign (argv.begin (), argv.end ());

    return m_supervisor.add_child (child);
  }

  void RifImpl::on_child_failed (gspc::rif::supervisor_t::child_info_t const &chld)
  {
    MLOG (WARN, "child failed: " << chld.descriptor.name);
  }
  void RifImpl::on_child_started (gspc::rif::supervisor_t::child_info_t const &chld)
  {
    MLOG (INFO, "child started: " << chld.descriptor.name);
  }
  void RifImpl::on_child_terminated (gspc::rif::supervisor_t::child_info_t const &chld)
  {
    MLOG (WARN, "child terminated: " << chld.descriptor.name);
  }

  void RifImpl::handle ( std::string const &dst
              , gspc::net::frame const &rqst
              , gspc::net::user_ptr user
              )
  {
    std::string const cmd (rqst.get_body ());

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
      gspc::rif::proc_t p = m_mgr.exec (argv);

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
        procs = m_mgr.processes ();
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
        int rc = m_mgr.wait (p, &status, boost::posix_time::seconds (0));
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
        procs = m_mgr.processes ();
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
        int rc = m_mgr.term (p, boost::posix_time::seconds (1));
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
        procs = m_mgr.processes ();
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
        int rc = m_mgr.kill (p, sig);
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
        procs = m_mgr.processes ();
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
        m_mgr.remove (p);
      }
    }
    else if (command == "ps")
    {
      gspc::rif::proc_list_t procs;
      if (argv.empty ())
      {
        procs = m_mgr.processes ();
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
        int rc = m_mgr.proc_info (p, info);

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
        procs = m_mgr.processes ();
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
          int rc = m_mgr.read ( p
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
        m_mgr.write (p, fd, argv [i].c_str (), argv [i].size (), ec);
        if (i < (argv.size () - 1))
          m_mgr.write (p, fd, " ", 1, ec);

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

      m_mgr.setenv (argv [0], argv [1]);
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
      if (0 == m_mgr.getenv (argv [0], val))
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

      m_mgr.delenv (argv [0]);
    }
    else if (command == "env")
    {
      gspc::rif::env_t env = m_mgr.env ();
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

      sstr << "GspcRIFD running on " << fhg::util::hostname() << " as " << getpid ()
           << std::endl
        ;

      rply.set_body (sstr.str ());
    }
    else if (command == "shutdown")
    {
      _request_shutdown();
    }
    else if (command == "supervise")
    {
      try
      {
        supervise (std::list<std::string>(argv.begin (), argv.end ()));
        rply.set_body ("OK");
      }
      catch (std::exception const &ex)
      {
        rply = gspc::net::make::error_frame ( rqst
                                            , gspc::net::E_SERVICE_FAILED
                                            , ex.what ()
                                            );
        user->deliver (rply);
        return;
      }
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
