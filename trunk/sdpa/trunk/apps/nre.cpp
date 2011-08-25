#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
//#include <unistd.h>
#include <csignal>
#include "sdpa/daemon/jobFSM/JobFSM.hpp"
#include <sdpa/sdpa-config.hpp>

#include <sdpa/logging.hpp>
#include <sdpa/util/Config.hpp>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

#include <sdpa/daemon/nre/NREFactory.hpp>
#include <sdpa/daemon/nre/NRE.hpp>
#include <seda/StageRegistry.hpp>
#include <sdpa/engine/RealWorkflowEngine.hpp>
#include <boost/filesystem/path.hpp>
#include <fhgcom/kvs/kvsc.hpp>


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

enum eBkOpt { NO_BKP=1, FILE_DEF, FLD_DEF, FLDANDFILE_DEF=6 };
const unsigned int MAX_CAP = 2;

int main (int argc, char **argv)
{
  FHGLOG_SETUP();

  string nreName;
  string nreUrl;
  vector<string> arrMasterNames;
  vector<string> arrMasterUrls;

  string workerUrl;
  string guiUrl;
  string kvsUrl;

  bool bDoBackup = false;
  std::string backup_file;
  std::string backup_folder;

  po::options_description desc("Allowed options");
  desc.add_options()
     ("help,h", "Display this message")
     ("name,n", po::value<std::string>(&nreName)->default_value("NRE_0"), "NRE's logical name")
     ("url,u",  po::value<std::string>(&nreUrl)->default_value("localhost"), "NRE's url")
     //("agg_name,m",  po::value<std::string>(&aggName)->default_value("aggregator"), "Aggregator's logical name")
     ("master,m", po::value<vector<string> >(&arrMasterNames)->multitoken(), "Nre's master list")
     //("agg_url,p",  po::value<std::string>(&aggUrl)->default_value("127.0.0.1:5001"), "Aggregator's url")
     ("worker_url,w",  po::value<std::string>(&workerUrl)->default_value("127.0.0.1:8000"), "Worker's url")
     ("gui_url,g",  po::value<std::string>(&guiUrl)->default_value("127.0.0.1:9000"), "GUI's url")
     ("backup_folder,d", po::value<std::string>(&backup_folder), "NRE's backup folder")
     ("backup_file,f", po::value<std::string>(&backup_file), "NRE's backup file")
     ("kvs_url,k",  po::value<string>(), "The kvs daemon's url")
    ("use-push-model", "use push model instead of request model")
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

  if( !vm.count("kvs_url") )
  {
          LOG(ERROR, "The url of the kvs daemon was not specified!");
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
                  LOG(ERROR, "Invalid kvs url.  Please specify it in the form <hostname (IP)>:<port>!");
                  return -1;
          }
          else
          {
                  LOG(INFO, "The kvs daemon is assumed to run at "<<vec[0]<<":"<<vec[1]);
                  fhg::com::kvs::global::get_kvs_info().init( vec[0], vec[1], boost::posix_time::seconds(10), 3);
          }
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
                  backup_file = nreName + ".bak";
                  LOG( WARN, "Backup file not specified! Backup the nre by default into "<<backup_file);
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
                          LOG(INFO, "Backup the nre into the file "<<backup_folder<<"/"<<backup_file );
                          bDoBackup = true;
                  }
                  break;

  case NO_BKP:

                  LOG( WARN, "No backup folder and no backup file were specified! No backup for the nre will be available!");
                  bDoBackup = false;
                  break;

  default:
                  LOG(ERROR, "Bad luck, This should not happen!");
                  bDoBackup = false;
  }


  if( arrMasterNames.empty() )
    arrMasterNames.push_back("aggregator"); // default master name

  sdpa::master_info_list_t listMasterInfo;
  LOG(INFO, "Starting the agent with the name = '"<<nreName<<"' at location "<<nreUrl<<", with the nre-worker running at "<<workerUrl
             <<"and having the masters: ");

  BOOST_FOREACH(string master, arrMasterNames)
  {
	  cout<<"   "<<master<<std::endl;
  	  listMasterInfo.push_back(sdpa::MasterInfo(master));
  }

  try {

	  sdpa::daemon::NRE<WorkerClient>::ptr_t ptrNRE
	     = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient >::create( nreName
										    , nreUrl
										    , listMasterInfo
										    , MAX_CAP
										    , workerUrl
									        , guiUrl);

	  bool bUseRequestModel(vm.count("use-push-model") == 0);

	  if(bDoBackup)
		  ptrNRE->start_agent(bUseRequestModel, bkp_path/backup_file);
	  else
		  ptrNRE->start_agent(bUseRequestModel);

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
    ptrNRE->shutdown();

  } catch (std::exception const & ex) {
    LOG(ERROR, "Could not start the NRE: " << ex.what() );
  }

  return 0;
}
