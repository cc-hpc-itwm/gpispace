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
#include <sdpa/daemon/aggregator/AggregatorFactory.hpp>
#include <sdpa/engine/RealWorkflowEngine.hpp>

namespace su = sdpa::util;
namespace po = boost::program_options;
using namespace std;

int main (int argc, char **argv)
{
	string aggName;
	string aggUrl;
	string orchName;
	string orchUrl;
	string backup_file;

	po::options_description desc("Allowed options");
	desc.add_options()
	   ("help", "Display this message")
	   ("name,n", po::value<std::string>(&aggName)->default_value("aggregator"), "Aggregator's logical name")
	   ("url,u",  po::value<std::string>(&aggUrl)->default_value("localhost"), "Aggregator's url")
	   ("orch_name,m",  po::value<std::string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
	   ("backup_file,f", po::value<std::string>(&backup_file)->default_value("./aggregator.bkp"), "Aggregator's backup file")
	   ;

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cerr << "usage: aggregator [options] ...." << std::endl;
		std::cerr << desc << std::endl;
		return 0;
	}

	std::cout <<"Starting the aggregator with the name = '"<<aggName<<"' at location "<<aggUrl<<std::endl
			  <<" having the master "<<orchName<<"("<<orchUrl<<")"<<std::endl;
	fhg::log::Configurator::configure();

	try {
		sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create( aggName, aggUrl, orchName); //, orchUrl );
		ptrAgg->start_agent(backup_file);

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

		ptrAgg->shutdown(backup_file);
	} catch ( std::exception& ){
			std::cout<<"Could not start the Aggregator!"<<std::endl;
		}

}
