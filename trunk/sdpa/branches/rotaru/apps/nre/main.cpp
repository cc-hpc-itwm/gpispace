#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unistd.h>

#include <sdpa/sdpa-config.hpp>

#include <sdpa/logging.hpp>
#include <sdpa/util/Config.hpp>

#include <boost/program_options.hpp>
#include <sdpa/daemon/nre/NRE.hpp>

namespace su = sdpa::util;
namespace po = boost::program_options;
using namespace std;

void outputUsage(const po::options_description& desc)
{
   std::cout << "<binary> -n <name> -u <url> -m <agg_name> -p <agg_url> -w <worker_url> "<<std::endl;
}

int main (int argc, char **argv)
{
	string nreName("NRE_0");
	string nreUrl("127.0.0.1:5002");
	string aggName("aggregator_1");
	string aggUrl("127.0.0.1:5001");
	string workerUrl("127.0.0.1:8000");

	po::options_description desc("Allowed options");
	desc.add_options()
	   ("help,h", "Display this message")
	   ("name,n", po::value<std::string>(&nreName)->default_value("NRE_0"), "NRE's logical name")
	   ("url,u",  po::value<std::string>(&nreUrl)->default_value("127.0.0.1:5002"), "NRE's url")
	   ("agg_name,m",  po::value<std::string>(&aggName)->default_value("aggregator_1"), "Aggregator's logical name")
	   ("agg_url,p",  po::value<std::string>(&aggUrl)->default_value("127.0.0.1:5001"), "Aggregator's url")
	   ("worker_url,w",  po::value<std::string>(&workerUrl)->default_value("127.0.0.1:8000"), "Worker's url");

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

	std::cout <<"Starting the NRE with the name = '"<<nreName<<"' at location "<<nreUrl<<std::endl
			  <<" having the master "<<aggName<<"("<<aggUrl<<")"<<std::endl
			  <<" with the nre-worker running at "<<workerUrl<<std::endl;

	fhg::log::Configurator::configure();

	try {
		sdpa::daemon::NRE::ptr_t ptrNRE = sdpa::daemon::NRE::create( nreName, nreUrl, aggName, aggUrl, workerUrl );
		sdpa::daemon::NRE::start(ptrNRE);

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

		sdpa::daemon::NRE::shutdown(ptrNRE);
	} catch( std::exception& ) {
			std::cout<<"Could not start the NRE!"<<std::endl;
		}

}
