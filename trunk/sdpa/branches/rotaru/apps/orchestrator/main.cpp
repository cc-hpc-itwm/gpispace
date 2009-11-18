#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unistd.h>

#include <sdpa/sdpa-config.hpp>

#include <sdpa/logging.hpp>
#include <sdpa/util/Config.hpp>

#include <boost/program_options.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>

namespace su = sdpa::util;
namespace po = boost::program_options;
using namespace std;

void outputUsage(const po::options_description& desc)
{
   std::cout << "<binary> -n <name> -u <url>"<<std::endl;
}

int main (int argc, char **argv)
{
	string orchName("orchestrator_0");
	string orchUrl("127.0.0.1:5000");

	po::options_description desc("Allowed options");
	desc.add_options()
	   ("help", "Display this message")
	   ("name,n", po::value<std::string>(&orchName), "Orchestrator's logical name")
	   ("url,u",  po::value<std::string>(&orchUrl)->default_value("127.0.0.1:5000"), "Orchestrator's url");

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

	try {
		sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create( orchName, orchUrl );
		sdpa::daemon::Orchestrator::start(ptrOrch);
	} catch( std::exception& ) {
		std::cout<<"Could not start the Orchestrator!"<<std::endl;
	}

	//sdpa::daemon::Orchestrator::shutdown(ptrOrch);
}
