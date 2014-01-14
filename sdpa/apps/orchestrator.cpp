#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <csignal>

#include <fhglog/fhglog.hpp>

#include <boost/program_options.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <boost/filesystem/path.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhg/util/daemonize.hpp>

#include <boost/tokenizer.hpp>

namespace bfs = boost::filesystem;
namespace po = boost::program_options;
using namespace std;

static const int EX_STILL_RUNNING = 4;

int main (int argc, char **argv)
{
    string orchName;
    string orchUrl;
    string kvsUrl;
    string pidfile;
    bool daemonize = false;

    FHGLOG_SETUP();

    po::options_description desc("Allowed options");
    desc.add_options()
       ("help,h", "Display this message")
       ("name,n", po::value<std::string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
       ("url,u",  po::value<std::string>(&orchUrl)->default_value("localhost"), "Orchestrator's url")
       ("kvs_url,k",  po::value<string>(), "The kvs daemon's url")
       ("pidfile", po::value<std::string>(&pidfile)->default_value(pidfile), "write pid to pidfile")
       ("daemonize", "daemonize after all checks were successful")
       ;

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    fhg::log::Logger::ptr_t logger (fhg::log::Logger::get (orchName));

    if (vm.count("help"))
    {
      LLOG (ERROR, logger, "usage: orchestrator [options] ....");
      LLOG (ERROR, logger, desc);
      return 0;
    }

    if( !vm.count("kvs_url") )
    {
      LLOG (ERROR, logger, "The url of the kvs daemon was not specified!");
      return -1;
    }
    else
    {
      boost::char_separator<char> sep(":");
      boost::tokenizer<boost::char_separator<char> > tok(vm["kvs_url"].as<std::string>(), sep);

      vector< string > vec;
      vec.assign(tok.begin(),tok.end());

      if( vec.size() != 2 )
      {
        LLOG (ERROR, logger, "Invalid kvs url.  Please specify it in the form <hostname (IP)>:<port>!");
        return -1;
      }
      else
      {
        fhg::com::kvs::global::get_kvs_info().init( vec[0], vec[1], boost::posix_time::seconds(120), 1);
      }
    }

    if (vm.count ("daemonize"))
      daemonize = true;

    int pidfile_fd = -1;

    if (not pidfile.empty())
    {
      pidfile_fd = open(pidfile.c_str(), O_CREAT|O_RDWR, 0640);
      if (pidfile_fd < 0)
      {
        LLOG (ERROR, logger, "could not open pidfile for writing: "<< strerror(errno));
        exit(EXIT_FAILURE);
      }
    }

    if (daemonize)
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent();
    }

    if (pidfile_fd >= 0)
    {
      if (lockf(pidfile_fd, F_TLOCK, 0) < 0)
      {
        LLOG (ERROR, logger, "could not lock pidfile: "<< strerror(errno));
        exit(EX_STILL_RUNNING);
      }

      char buf[32];
      ftruncate(pidfile_fd, 0);
      snprintf(buf, sizeof(buf), "%d\n", getpid());
      write(pidfile_fd, buf, strlen(buf));
      fsync(pidfile_fd);
    }

    try {
      const sdpa::daemon::Orchestrator orchestrator (orchName, orchUrl);

      DLLOG (TRACE, logger, "waiting for signals...");
      sigset_t waitset;
      int sig(0);
      int result(0);

      sigfillset(&waitset);
      sigprocmask(SIG_BLOCK, &waitset, NULL);

      bool signal_ignored = true;
      while (signal_ignored)
      {
        result = sigwait(&waitset, &sig);
        if (result == 0)
        {
          DLLOG (TRACE, logger, "got signal: " << sig);
          switch (sig)
          {
            case SIGTERM:
            case SIGINT:
              signal_ignored = false;
              break;
            default:
              LLOG (INFO, logger, "ignoring signal: " << sig);
              break;
          }
        }
        else
        {
          LLOG (ERROR, logger, "error while waiting for signal: " << result);
        }
      }

      DLLOG (TRACE, logger, "terminating...");
    }
    catch( std::exception& )
    {
      LLOG (ERROR, logger, "Could not start the Orchestrator!");
    }
}
