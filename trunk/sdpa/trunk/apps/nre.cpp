#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
//#include <unistd.h>
#include <csignal>

#include <sdpa/sdpa-config.hpp>

#include <sdpa/logging.hpp>
#include <sdpa/util/Config.hpp>

#include <boost/program_options.hpp>
#include <sdpa/daemon/nre/NREFactory.hpp>
#include <sdpa/daemon/nre/NRE.hpp>
#include <seda/StageRegistry.hpp>
#include <sdpa/engine/RealWorkflowEngine.hpp>
#include <boost/filesystem/path.hpp>


#ifdef USE_REAL_WE
	#include <sdpa/daemon/nre/nre-worker/NreWorkerClient.hpp>
	typedef sdpa::nre::worker::NreWorkerClient WorkerClient;
#else
	#include <sdpa/daemon/nre/BasicWorkerClient.hpp>
	typedef sdpa::nre::worker::BasicWorkerClient WorkerClient;
#endif

namespace su = sdpa::util;
namespace bfs = boost::filesystem;
namespace po = boost::program_options;
using namespace std;

int main (int argc, char **argv)
{
  FHGLOG_SETUP();

	string nreName;
	string nreUrl;
	string aggName;
	string aggUrl;
	string workerUrl;
	string guiUrl;
	bfs::path backup_file;

	po::options_description desc("Allowed options");
	desc.add_options()
	   ("help,h", "Display this message")
	   ("name,n", po::value<std::string>(&nreName)->default_value("NRE_0"), "NRE's logical name")
	   ("url,u",  po::value<std::string>(&nreUrl)->default_value("localhost"), "NRE's url")
	   ("agg_name,m",  po::value<std::string>(&aggName)->default_value("aggregator"), "Aggregator's logical name")
	   //("agg_url,p",  po::value<std::string>(&aggUrl)->default_value("127.0.0.1:5001"), "Aggregator's url")
	   ("worker_url,w",  po::value<std::string>(&workerUrl)->default_value("127.0.0.1:8000"), "Worker's url")
	   ("gui_url,g",  po::value<std::string>(&guiUrl)->default_value("127.0.0.1:9000"), "GUI's url")
	   ("backup_file,f","NRE's backup file")
	   ;

  po::variables_map vm;
  //po::store(po::parse_command_line(argc, argv, desc), vm);
  po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cerr << "usage: nre [options] ...." << std::endl;
    std::cerr << desc << std::endl;
    return 0;
  }

  if (vm.count("backup_file"))
  {
	  LOG(INFO, "Backup the NRE into the file "<< vm["backup_file"].as<string>() );
  }
  else
  {
	  backup_file = nreName + "bak";
	  LOG(INFO, "Backup file for the NRE not explicitly specified. Backup it by default into "<<  backup_file.string() );
  }

  LOG(INFO,	"Starting the NRE with the name = '"<<nreName<<"' at location "<<nreUrl<<", having the master "<<aggName<<"("<<aggUrl<<")"
		  	 <<", with the nre-worker running at "<<workerUrl);

  sdpa::daemon::NRE<WorkerClient>::ptr_t ptrNRE
    = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient >::create( nreName
																	, nreUrl
																	, aggName
																	, workerUrl
																	, guiUrl
																	);

  try {
	  ptrNRE->start_agent(backup_file);

    LOG(DEBUG, "waiting for signals...");
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
        LOG(DEBUG, "got signal: " << sig);
        switch (sig)
        {
          case SIGTERM:
          case SIGINT:
            signal_ignored = false;
            break;
          default:
            LOG(DEBUG, "ignoring signal: " << sig);
            break;
        }
      }
      else
      {
        LOG(ERROR, "error while waiting for signal: " << result);
      }
    }

    LOG(INFO, "terminating...");
  } catch (std::exception const & ex) {
    LOG(ERROR, "Could not start the NRE: " << ex.what() );
  }

  ptrNRE->shutdown();

  seda::StageRegistry::instance().stopAll();
  seda::StageRegistry::instance().clear();

  return 0;
}
