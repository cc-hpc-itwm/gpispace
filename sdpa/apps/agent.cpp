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

#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/engine/RealWorkflowEngine.hpp>
#include <boost/filesystem/path.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhg/util/read_bool.hpp>

namespace bfs = boost::filesystem;
namespace su = sdpa::util;
namespace po = boost::program_options;

enum eBkOpt { NO_BKP=1, FILE_DEF, FLD_DEF, FLDANDFILE_DEF=6 };
const unsigned int MAX_CAP = 10000;

static const int EX_STILL_RUNNING = 4;

int main (int argc, char **argv)
{
        string agentName;
        string agentUrl;
        vector<string> arrMasterNames;
        string arrMasterUrls;
        string appGuiUrl;
        string kvsUrl;
        unsigned int agentRank;
        string pidfile;
        bool daemonize = false;

        bool bDoBackup = false;
        string backup_file;
        string backup_folder;
        string requestMode ("false");

        FHGLOG_SETUP();

        po::options_description desc("Allowed options");
        desc.add_options()
           ("help,h", "Display this message")
           ("name,n", po::value<string>(&agentName)->default_value("agent"), "Agent's logical name")
           ("url,u",  po::value<string>(&agentUrl)->default_value("localhost"), "Agent's url")
           //("orch_name,m",  po::value<string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
           ("master,m", po::value<vector<string> >(&arrMasterNames)->multitoken(), "Agent's master list")
           ("rank,r", po::value<unsigned int>(&agentRank)->default_value(0), "Agent's rank")
           ("backup_folder,d", po::value<string>(&backup_folder), "Agent's backup folder")
           ("backup_file,f", po::value<string>(&backup_file), "Agent's backup file (stored into the backup folder)")
           ("app_gui_url,a", po::value<string>(&appGuiUrl)->default_value("127.0.0.1:9000"), "application GUI's url")
           ("kvs_url,k",  po::value<string>(), "The kvs daemon's url")
           ("request-mode", po::value<string>(&requestMode)->default_value(requestMode), "send periodical job requests to master")
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
            boost::tokenizer<boost::char_separator<char> > tok(vm["kvs_url"].as<string>(), sep);

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
                fhg::com::kvs::global::get_kvs_info().init( vec[0], vec[1], boost::posix_time::seconds(5), 1);
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
            backup_file = agentName + ".bak";
            LOG( INFO, "Backup file not specified! Backup the agent by default into "<<backup_file);
            // check if the folder exists
            if( !bfs::is_directory(st) )             // true - is directory
            {
              LOG(ERROR, "The path "<<backup_folder<<" does not represent a folder!" );
              bDoBackup = false;
            }
            else
            {
              LOG(INFO, "Backup the agent into the file "<<backup_folder<<"/"<<backup_file );
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
              LOG(INFO, "Backup the agent into the file "<<backup_folder<<"/"<<backup_file );
              bDoBackup = true;
            }
            break;

        case NO_BKP:
            MLOG (INFO, "No backup folder and no backup file were specified! No backup for the agent will be available!");
            bDoBackup = false;
            break;

        default:
            LOG(ERROR, "Bad luck, This should not happen!");
            bDoBackup = false;
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

        LOG(INFO, "Starting the agent with the name = '"<<agentName<<"' at location "<<agentUrl<<", having the masters: ");
        BOOST_FOREACH(string master, arrMasterNames)
        {
           cout<<"   "<<master<<std::endl;
           listMasterInfo.push_back(sdpa::MasterInfo(master));
        }

        try
        {
          sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<RealWorkflowEngine>::create(   agentName,
                                                                                                          agentUrl,
                                                                                                          listMasterInfo,
                                                                                                          MAX_CAP,
                                                                                                          false,
                                                                                                          agentRank,
                                                                                                          appGuiUrl ); //, orchUrl );

          bool bUseRequestModel(fhg::util::read_bool(requestMode));

          if(bDoBackup)
            ptrAgent->start_agent(bUseRequestModel, bkp_path/backup_file);
          else
            ptrAgent->start_agent(bUseRequestModel);

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

          ptrAgent->shutdown();
        }
        catch ( std::exception& )
        {
          std::cout<<"Could not start the Agent!"<<std::endl;
        }

}
