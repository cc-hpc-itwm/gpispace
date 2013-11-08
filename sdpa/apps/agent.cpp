#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <csignal>

#include <sdpa/sdpa-config.hpp>

#include <sdpa/logging.hpp>
#include <sdpa/util/Config.hpp>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <we/mgmt/layer.hpp>
#include <boost/filesystem/path.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhg/util/read_bool.hpp>

#include <fhg/util/stat.hpp>

#include <boost/tokenizer.hpp>

namespace bfs = boost::filesystem;
namespace su = sdpa::util;
namespace po = boost::program_options;

static const int EX_STILL_RUNNING = 4;

int main (int argc, char **argv)
{
  std::string agentName;
  std::string agentUrl;
  std::vector<std::string> arrMasterNames;
  std::string arrMasterUrls;
  std::string appGuiUrl;
  std::string kvsUrl;
  unsigned int agentRank;
  std::string pidfile;

  FHGLOG_SETUP();

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "Display this message")
    ("name,n", po::value<std::string>(&agentName)->default_value("agent"), "Agent's logical name")
    ("url,u",  po::value<std::string>(&agentUrl)->default_value("localhost"), "Agent's url")
    //("orch_name,m",  po::value<std::string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
    ("master,m", po::value<std::vector<std::string> >(&arrMasterNames)->multitoken(), "Agent's master list")
    ("rank,r", po::value<unsigned int>(&agentRank)->default_value(0), "Agent's rank")
    ("app_gui_url,a", po::value<std::string>(&appGuiUrl)->default_value("127.0.0.1:9000"), "application GUI's url")
    ("kvs_url,k",  po::value<std::string>(), "The kvs daemon's url")
    ("pidfile", po::value<std::string>(&pidfile)->default_value(pidfile), "write pid to pidfile")
    ("daemonize", "daemonize after all checks were successful")
    ;

  po::variables_map vm;
  po::store( po::command_line_parser( argc, argv ).options(desc).run(), vm );
  po::notify(vm);

  if( vm.count("help") )
  {
    std::cerr << "usage: agent [options] ...." << std::endl;
    std::cerr << desc << std::endl;
    return 0;
  }

  if( !vm.count("kvs_url") )
  {
    LOG(ERROR, "The url of the kvs daemon was not specified!");
    return -1;
  }
  else
  {
    boost::char_separator<char> sep(":");
    boost::tokenizer<boost::char_separator<char> > tok(vm["kvs_url"].as<std::string>(), sep);

    std::vector< std::string > vec;
    vec.assign(tok.begin(),tok.end());

    if( vec.size() != 2 )
    {
      LOG(ERROR, "Invalid kvs url.  Please specify it in the form <hostname (IP)>:<port>!");
      return -1;
    }
    else
    {
      DMLOG(TRACE, "The kvs daemon is assumed to run at "<<vec[0]<<":"<<vec[1]);
      fhg::com::kvs::global::get_kvs_info().init( vec[0], vec[1], boost::posix_time::seconds(120), 1);
    }
  }

  if( arrMasterNames.empty() )
    arrMasterNames.push_back("orchestrator"); // default master name

  sdpa::master_info_list_t listMasterInfo;

  int pidfile_fd = -1;

  if (not pidfile.empty())
  {
    pidfile_fd = open(pidfile.c_str(), O_CREAT|O_RDWR, 0640);
    if (pidfile_fd < 0)
    {
      LOG( ERROR, "could not open pidfile for writing: "
         << strerror(errno)
         );
      exit(EXIT_FAILURE);
    }
  }

  // everything is fine so far, daemonize
  if (vm.count ("daemonize"))
  {
    if (pid_t child = fork())
    {
      if (child == -1)
      {
        LOG(ERROR, "could not fork: " << strerror(errno));
        exit(EXIT_FAILURE);
      }
      else
      {
        exit (EXIT_SUCCESS);
      }
    }
    setsid();
    close(0); close(1); close(2);
    int fd = open("/dev/null", O_RDWR);
    if (-1 == dup(fd))
    {
      LOG(WARN, "could not duplicate /dev/null to stdout: " << strerror(errno));
    }
    if (-1 == dup(fd))
    {
      LOG(WARN, "could not duplicate /dev/null to stdout: " << strerror(errno));
    }
  }

  if (pidfile_fd >= 0)
  {
    if (lockf(pidfile_fd, F_TLOCK, 0) < 0)
    {
      LOG( ERROR, "could not lock pidfile: "
         << strerror(errno)
         );
      exit(EX_STILL_RUNNING);
    }

    char buf[32];
    if (0 != ftruncate(pidfile_fd, 0))
    {
      LOG(WARN, "could not truncate pidfile: " << strerror(errno));
    }
    snprintf(buf, sizeof(buf), "%d\n", getpid());
    if (write(pidfile_fd, buf, strlen(buf)) <= 0)
    {
      LOG(ERROR, "could not write pid: " << strerror(errno));
      exit(EXIT_FAILURE);
    }
    fsync(pidfile_fd);
  }

  {
    std::stringstream startup_message;
    startup_message << "Starting agent '" << agentName
                    << "' at '" << agentUrl
                    << "', having masters: ";

    BOOST_FOREACH (const std::string& master, arrMasterNames)
    {
      startup_message << master << ", ";
      listMasterInfo.push_back (sdpa::MasterInfo (master));
    }

    DMLOG (TRACE, startup_message.str());
  }

  try
  {
    sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create_with_start_called (   agentName,
                                                                                             agentUrl,
                                                                                             listMasterInfo,
                                                                                             agentRank,
                                                                                             appGuiUrl ); //, orchUrl );

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
        DMLOG (TRACE, "got signal: " << sig);
        switch (sig)
        {
        case SIGTERM:
        case SIGINT:
          signal_ignored = false;
          break;
        default:
          DMLOG (TRACE, "ignoring signal: " << sig);
          break;
        }
      }
      else
      {
        LOG(ERROR, "error while waiting for signal: " << result);
      }
    }

    DMLOG(TRACE, "terminating...");

    {
      std::ostringstream oss;

      FHG_UTIL_STAT_OUT (oss);

      if (oss.str().size())
      {
        DMLOG (DEBUG, oss.str());
      }
    }

    ptrAgent->shutdown();
  }
  catch ( std::exception& )
  {
    LOG (FATAL, "Could not start the Agent!");
  }
}
