#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <csignal>

#include <fhglog/fhglog.hpp>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

#include <sdpa/daemon/agent/Agent.hpp>
#include <we/layer.hpp>
#include <boost/filesystem/path.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhg/util/read_bool.hpp>
#include <fhg/util/daemonize.hpp>
#include <fhg/util/pidfile_writer.hpp>

#include <boost/tokenizer.hpp>

namespace bfs = boost::filesystem;
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

  fhg::log::Logger::ptr_t logger (fhg::log::Logger::get (agentName));

  if( vm.count("help") )
  {
    LLOG (ERROR, logger, "usage: agent [options] ....");
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

    std::vector< std::string > vec;
    vec.assign(tok.begin(),tok.end());

    if( vec.size() != 2 )
    {
      LLOG (ERROR, logger, "Invalid kvs url.  Please specify it in the form <hostname (IP)>:<port>!");
      return -1;
    }
    else
    {
      DLLOG (TRACE, logger, "The kvs daemon is assumed to run at "<<vec[0]<<":"<<vec[1]);
      fhg::com::kvs::global::get_kvs_info().init( vec[0], vec[1], boost::posix_time::seconds(120), 1);
    }
  }

  if( arrMasterNames.empty() )
    arrMasterNames.push_back("orchestrator"); // default master name

  sdpa::master_info_list_t listMasterInfo;

  if (not pidfile.empty())
  {
    fhg::util::pidfile_writer const pidfile_writer (pidfile);

    if (vm.count ("daemonize"))
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent();
    }

    pidfile_writer.write();
  }
  else
  {
    if (vm.count ("daemonize"))
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent();
    }
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

    DLLOG (TRACE, logger, startup_message.str());
  }

    const sdpa::daemon::Agent agent
      (agentName, agentUrl, listMasterInfo, agentRank, appGuiUrl);

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
          DLLOG (TRACE, logger, "ignoring signal: " << sig);
          break;
        }
      }
      else
      {
        LLOG (ERROR, logger, "error while waiting for signal: " << result);
      }
    }
}
