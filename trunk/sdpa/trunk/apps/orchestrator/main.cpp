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
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/engine/RealWorkflowEngine.hpp>

typedef void WorkflowEngineType;

namespace su = sdpa::util;
namespace po = boost::program_options;
using namespace std;

int main (int argc, char **argv)
{
	string orchName;
	string orchUrl;
	string backup_file;

	po::options_description desc("Allowed options");
	desc.add_options()
	   ("help", "Display this message")
	   ("name,n", po::value<std::string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
	   ("url,u",  po::value<std::string>(&orchUrl)->default_value("localhost"), "Orchestrator's url")
	   ("backup_file,f", po::value<std::string>(&backup_file)->default_value("./orchestrator.bkp"), "Orchestrator's backup file")
	   ;

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cerr << "usage: orchestrator [options] ...." << std::endl;
		std::cerr << desc << std::endl;
		return 0;
	}

	std::cout <<"Starting the orchestrator with the name = '"<<orchName<<"' at location "<<orchUrl<<std::endl;

	FHGLOG_SETUP();
	//	fhg::log::Configurator::configure();

	try {
		sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<WorkflowEngineType>::create( orchName, orchUrl  );
		ptrOrch->start_agent(backup_file);

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
					LOG(INFO, "ignoring signal: " << sig);
					break;
				}
			}
			else
			{
				LOG(ERROR, "error while waiting for signal: " << result);
			}
		}

		LOG(INFO, "terminating...");

		ptrOrch->shutdown(backup_file);
	} catch( std::exception& ) {
			std::cout<<"Could not start the Orchestrator!"<<std::endl;
		}
}
