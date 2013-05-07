#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
//#include <unistd.h>
#include <csignal>
#include "sdpa/daemon/JobFSM.hpp"
#include <sdpa/sdpa-config.hpp>

#include <sdpa/logging.hpp>
#include <sdpa/util/Config.hpp>

#include <boost/program_options.hpp>
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <boost/filesystem/path.hpp>
#include <fhgcom/kvs/kvsc.hpp>

namespace bfs = boost::filesystem;
namespace su = sdpa::util;
namespace po = boost::program_options;
using namespace std;

enum eBkOpt { NO_BKP=1, FILE_DEF, FLD_DEF, FLDANDFILE_DEF=6 };
const unsigned int MAX_CAP = 10000;

static const int EX_STILL_RUNNING = 4;

int main (int argc, char **argv)
{
    string orchName;
    string orchUrl;
    string kvsUrl;
    string pidfile;
    bool daemonize = false;

    bool bDoBackup = false;
    std::string backup_file;
    std::string backup_folder;

    FHGLOG_SETUP();

    po::options_description desc("Allowed options");
    desc.add_options()
       ("help,h", "Display this message")
       ("name,n", po::value<std::string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
       ("url,u",  po::value<std::string>(&orchUrl)->default_value("localhost"), "Orchestrator's url")
       ("backup_folder,d", po::value<std::string>(&backup_folder), "Orchestrator's backup folder")
       ("backup_file,f", po::value<std::string>(&backup_file), "Orchestrator's backup file")
       ("kvs_url,k",  po::value<string>(), "The kvs daemon's url")
       ("pidfile", po::value<std::string>(&pidfile)->default_value(pidfile), "write pid to pidfile")
       ("daemonize", "daemonize after all checks were successful")
       //("use-push-model", "use push model instead of request model")
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
        fhg::com::kvs::global::get_kvs_info().init( vec[0], vec[1], boost::posix_time::seconds(120), 1);
      }
    }

    int bkpOpt = NO_BKP;
    if( vm.count("backup_file") )
      bkpOpt *= FILE_DEF;

    if( vm.count("backup_folder") )
      bkpOpt *= FLD_DEF;
    if (vm.count ("daemonize"))
      daemonize = true;

    bfs::path bkp_path(backup_folder);
    boost::filesystem::file_status st = boost::filesystem::status(bkp_path);

    switch(bkpOpt)
    {
      case FLD_DEF:
              backup_file = orchName + ".bak";
              LOG( WARN, "Backup file not specified! Backup the orchestrator by default into "<<backup_file);
              // check if the folder exists
              if( !bfs::is_directory(st) )             // true - is directory
              {
                LOG(FATAL, "The path "<<backup_folder<<" does not represent a folder!" );
                bDoBackup = false;
              }
              else
              {
                LOG(INFO, "Backup the orchestrator into the file "<<backup_folder<<"/"<<backup_file );
                bDoBackup = true;
              }
              break;

      case FILE_DEF:
              LOG( INFO, "Backup folder not specified! No backup file will be created!");
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
                LOG(INFO, "Backup the orchestrator into the file "<<backup_folder<<"/"<<backup_file );
                bDoBackup = true;
              }
              break;

      case NO_BKP:
              bDoBackup = false;
              break;

      default:
              LOG(ERROR, "Bad luck, This should not happen!");
              bDoBackup = false;
    }

    int pidfile_fd = -1;

    if (not pidfile.empty())
    {
      pidfile_fd = open(pidfile.c_str(), O_CREAT|O_RDWR, 0640);
      if (pidfile_fd < 0)
      {
        LOG( ERROR, "could not open pidfile for writing: "<< strerror(errno));
        exit(EXIT_FAILURE);
      }
    }

    // everything is fine so far, daemonize
    if (daemonize)
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
      dup(fd);
      dup(fd);
    }

    if (pidfile_fd >= 0)
    {
      if (lockf(pidfile_fd, F_TLOCK, 0) < 0)
      {
        LOG( ERROR, "could not lock pidfile: "<< strerror(errno));
        exit(EX_STILL_RUNNING);
      }

      char buf[32];
      ftruncate(pidfile_fd, 0);
      snprintf(buf, sizeof(buf), "%d\n", getpid());
      write(pidfile_fd, buf, strlen(buf));
      fsync(pidfile_fd);
    }

    try {
      sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create( orchName, orchUrl, MAX_CAP  );

      bool bUseRequestModel = false; //(vm.count("use-push-model") == 0);

      if(bDoBackup)
        ptrOrch->start_agent(bUseRequestModel, bkp_path/backup_file);
      else
        ptrOrch->start_agent(bUseRequestModel);

      DMLOG (TRACE, "waiting for signals...");
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
              LOG(INFO, "ignoring signal: " << sig);
              break;
          }
        }
        else
        {
          LOG(ERROR, "error while waiting for signal: " << result);
        }
      }

      DMLOG (TRACE, "terminating...");

      ptrOrch->shutdown();
    }
    catch( std::exception& )
    {
      std::cout<<"Could not start the Orchestrator!"<<std::endl;
    }
}
