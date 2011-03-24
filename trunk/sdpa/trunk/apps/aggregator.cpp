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
#include <boost/filesystem/path.hpp>

namespace bfs = boost::filesystem;
namespace su = sdpa::util;
namespace po = boost::program_options;
using namespace std;

enum eBkOpt { NO_BKP=1, FILE_DEF, FLD_DEF, FLDANDFILE_DEF=6 };

int main (int argc, char **argv)
{
	string aggName;
	string aggUrl;
	string orchName;
	string orchUrl;
	string appGuiUrl;

	bool bDoBackup = false;
	std::string backup_file;
	std::string backup_folder;

	FHGLOG_SETUP();

	po::options_description desc("Allowed options");
	desc.add_options()
	   ("help", "Display this message")
	   ("name,n", po::value<std::string>(&aggName)->default_value("aggregator"), "Aggregator's logical name")
	   ("url,u",  po::value<std::string>(&aggUrl)->default_value("localhost"), "Aggregator's url")
	   ("orch_name,m",  po::value<std::string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
	   ("backup_folder,d", po::value<std::string>(&backup_folder), "Aggregator's backup folder")
	   ("backup_file,f", po::value<std::string>(&backup_file), "Aggregator's backup file (stored into the backup folder)")
	   ("app_gui_url,a",  po::value<std::string>(&appGuiUrl)->default_value("127.0.0.1:9000"), "application GUI's url")
	   ;

	po::variables_map vm;
	po::store( po::command_line_parser(argc, argv).options(desc).run(), vm );
	po::notify(vm);

	if( vm.count("help") )
	{
		std::cerr << "usage: aggregator [options] ...." << std::endl;
		std::cerr << desc << std::endl;
		return 0;
	}

	int bkpOpt = NO_BKP;
	if( vm.count("backup_file") )
		bkpOpt *= FILE_DEF;

	if( vm.count("backup_folder") )
		bkpOpt *= FLD_DEF;

	bfs::path bkp_path(backup_folder);
	boost::filesystem::file_status st = boost::filesystem::status(bkp_path);

	switch(bkpOpt)
	{
	case FLD_DEF:

			backup_file = aggName + ".bak";
			LOG( WARN, "Backup file not specified! Backup the aggregator by default into "<<backup_file);
			// check if the folder exists
			if( !bfs::is_directory(st) )             // true - is directory
			{
				LOG(FATAL, "The path "<<backup_folder<<" does not represent a folder!" );
				bDoBackup = false;
			}
			else
			{
				LOG(INFO, "Backup the aggregator into the file "<<backup_folder<<"/"<<backup_file );
				bDoBackup = true;
			}
			break;

	case FILE_DEF:
			LOG( WARN, "Backup folder not specified! No backup file will be created!");
			bDoBackup = false;
			break;

	case FLDANDFILE_DEF:
			LOG(INFO, "The backup folder is set to "<<backup_folder );

			// check if the folder exists
			if( !bfs::is_directory(st) )             // true - is directory
			{
				LOG(FATAL, "The path "<<backup_folder<<" does not represent a folder!" );
				bDoBackup = false;
			}
			else
			{
				LOG(INFO, "Backup the aggregator into the file "<<backup_folder<<"/"<<backup_file );
				bDoBackup = true;
			}
			break;

	case NO_BKP:

			LOG( WARN, "No backup folder and no backup file were specified! No backup for the aggregator will be available!");
			bDoBackup = false;
			break;

	default:
			LOG(ERROR, "Bad luck, This should not happen!");
			bDoBackup = false;
	}


	LOG(INFO, "Starting the aggregator with the name = '"<<aggName<<"' at location "<<aggUrl<<", having the master "<<orchName<<"("<<orchUrl<<")");

	try {
		sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create( aggName, aggUrl, orchName, appGuiUrl ); //, orchUrl );

		if(bDoBackup)
			ptrAgg->start_agent(bkp_path/backup_file);
		else
			ptrAgg->start_agent();

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

		ptrAgg->shutdown();
	} catch ( std::exception& ) {
		std::cout<<"Could not start the Aggregator!"<<std::endl;
	}

}
