#include "rif.hpp"

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <algorithm>
#include <cctype>

#include <boost/foreach.hpp>

#include <signal.h>

#include <gspc/net.hpp>
#include <gspc/rif.hpp>

class RifImpl;
static RifImpl *s_rif = 0;

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

  if (argv.empty ())
  {
    user->deliver (gspc::net::make::error_frame ( rqst
                                                , gspc::net::E_SERVICE_FAILED
                                                , "empty commandline"
                                                )
                  );
    return;
  }

  std::string command = argv [0];
  argv.erase (argv.begin ());
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
    if (argv.empty ())
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "no process specified"
                                          );
      user->deliver (rply);
      return;
    }

    gspc::rif::proc_t p;
    try
    {
      p = boost::lexical_cast<gspc::rif::proc_t> (*argv.begin ());
    }
    catch (boost::bad_lexical_cast const&)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "invalid proc"
                                          );
      user->deliver (rply);
      return;
    }

    int status;
    int rc = s_rif->mgr ().wait (p, &status, boost::posix_time::seconds (0));
    if (rc < 0)
    {
      std::stringstream sstr;
      sstr << "running: " << p << std::endl;
      rply.add_body (sstr.str ());
    }
    else
    {
      std::stringstream sstr;
      sstr << "terminated: " << p << ": " <<  gspc::rif::make_exit_code (status)
           << std::endl;
      rply.add_body (sstr.str ());
    }
  }
  else if (command == "term")
  {
    if (argv.empty ())
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "no process specified"
                                          );
      user->deliver (rply);
      return;
    }

    gspc::rif::proc_t p;
    try
    {
      p = boost::lexical_cast<gspc::rif::proc_t> (*argv.begin ());
    }
    catch (boost::bad_lexical_cast const&)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "invalid proc"
                                          );
      user->deliver (rply);
      return;
    }

    int rc = s_rif->mgr ().term (p, boost::posix_time::seconds (5));

    std::cerr << "term(" << p << ") = " << rc << std::endl;

    if (rc < 0)
    {
      std::stringstream sstr;
      sstr << "failed: " << -rc << ": " << strerror (-rc);
      user->deliver (gspc::net::make::error_frame ( rqst
                                                  , gspc::net::E_SERVICE_FAILED
                                                  , sstr.str ()
                                                  )
                    );
      return;
    }
  }
  else if (command == "kill")
  {
    if (argv.size () < 2)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "usage: kill SIG proc..."
                                          );
      user->deliver (rply);
      return;
    }

    int sig;
    try
    {
      sig = boost::lexical_cast<int> (argv [0]);
    }
    catch (boost::bad_lexical_cast const&)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "usage: kill SIG proc..."
                                          );
      user->deliver (rply);
      return;
    }

    for (size_t i = 1 ; i < argv.size () ; ++i)
    {
      gspc::rif::proc_t p;
      try
      {
        p = boost::lexical_cast<gspc::rif::proc_t> (argv [i]);
      }
      catch (boost::bad_lexical_cast const&)
      {
        std::stringstream sstr;
        sstr << "invalid proc: " << argv [i] << std::endl;
        continue;
      }

      int rc = s_rif->mgr ().kill (p, sig);
      if (rc < 0)
      {
        std::stringstream sstr;
        sstr << "failed on: " << p << ": " << strerror (-rc) << std::endl;
        rply.add_body (sstr.str ());
      }
      else
      {
        int status;
        rc = s_rif->mgr ().wait (p, &status, boost::posix_time::milliseconds (500));
        std::stringstream sstr;
        sstr << "terminated: " << p << ": " << gspc::rif::make_exit_code (status) << std::endl;
        rply.add_body (sstr.str ());
      }
    }
  }
  else if (command == "remove")
  {
    if (argv.size () < 1)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "usage: remove proc..."
                                          );
      user->deliver (rply);
      return;
    }

    for (size_t i = 0 ; i < argv.size () ; ++i)
    {
      gspc::rif::proc_t p;
      try
      {
        p = boost::lexical_cast<gspc::rif::proc_t> (argv [i]);
      }
      catch (boost::bad_lexical_cast const&)
      {
        std::stringstream sstr;
        sstr << "invalid proc: " << argv [i] << std::endl;
        continue;
      }

      int rc = s_rif->mgr ().remove (p);
      if (rc < 0)
      {
        std::stringstream sstr;
        sstr << "failed on: " << p << ": " << strerror (-rc) << std::endl;
        rply.add_body (sstr.str ());
      }
      else
      {
        std::stringstream sstr;
        sstr << "removed: " << p << std::endl;
        rply.add_body (sstr.str ());
      }
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
             << "\t" << info.argv ().front ()
             << std::endl
          ;
      }
    }
    rply.set_body (sstr.str ());
  }
  else if (command == "read")
  {
    if (argv.size () != 2)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "usage: read proc FD"
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

    int rc;
    char buf [4096];
    boost::system::error_code ec;
    std::stringstream sstr;
    do
    {
      rc = s_rif->mgr ().read (p, fd, buf, sizeof(buf) - 1, ec);
      if (rc > 0)
      {
        buf [rc] = 0;
        sstr << std::string (&buf[0], rc);
      }
    }
    while (rc > 0);

    rply.add_body (sstr.str ());
  }
  else if (command == "write")
  {
    if (argv.size () != 3)
    {
      rply = gspc::net::make::error_frame ( rqst
                                          , gspc::net::E_SERVICE_FAILED
                                          , "usage: write proc FD 'data'"
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

    boost::system::error_code ec;
    std::stringstream sstr;

    s_rif->mgr ().write (p, fd, argv [2].c_str (), argv [2].size (), ec);
    rply.set_body ("");
  }
  else
  {
    std::stringstream sstr;
    sstr << "usage: rif command <args...>" << std::endl
         << std::endl
         << " commands" << std::endl
         << "   exec command <args...>" << std::endl
         << "   wait proc" << std::endl
         << "   term proc" << std::endl
         << "   kill proc <signal>" << std::endl
         << "   read proc FD" << std::endl
         << "   write proc FD data" << std::endl
         << "   remove proc" << std::endl
         << "   ps [pid]" << std::endl
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
