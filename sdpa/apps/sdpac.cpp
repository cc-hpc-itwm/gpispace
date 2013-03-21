#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
//#include <unistd.h>
#include <sys/stat.h> // for stat, to check if file exists

#if defined(HAVE_CONFIG_H)
#include <sdpa/sdpa-config.hpp>
#endif

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>
#include <fhglog/FileAppender.hpp>
#include <fhglog/StreamAppender.hpp>

#include <fhg/util/getenv.hpp>

#include <sdpa/job_states.hpp>
#include <sdpa/client/ClientApi.hpp>
#include <seda/IEvent.hpp>
#include <sdpa/util/util.hpp>
#include <sdpa/util/Config.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/tokenizer.hpp>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <fhgcom/kvs/kvsc.hpp>

#include <fhg/error_codes.hpp>
#include <fhg/revision.hpp>

#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>

namespace fs = boost::filesystem;

enum return_codes_t
  {
    ERR_USAGE             = 50
  , JOB_ID_MISSING        = 51
  , FILE_EXISTS           = 52
  , IO_ERROR              = 53
  , NETWORK_ERROR         = 54
  , UNKNOWN_ERROR         = 100
  };

const int NMAXTRIALS = 10;

void get_user_input(std::string const & prompt, std::string & result, std::istream & in = std::cin)
{
  std::cout << prompt;
  std::string tmp;
  std::getline (in, tmp);
  if (tmp.size())
    result = tmp;
}

/* returns sdpa::status::code */
int command_poll_and_wait ( const std::string &job_id
                 , const sdpa::client::ClientApi::ptr_t &api
                 , boost::posix_time::time_duration poll_interval
                 , sdpa::client::job_info_t & job_info
                 )
{
  typedef boost::posix_time::ptime time_type;
  time_type poll_start = boost::posix_time::microsec_clock::local_time();

  std::cerr << "starting at: " << poll_start << std::endl;

  std::cerr << "waiting for job to return..." << std::flush;

  int status = sdpa::status::UNKNOWN;
  std::size_t fail_count(0);
  for (; fail_count < 3 ;)
  {
    try
    {
      status = api->queryJob(job_id, job_info);
      fail_count = 0; // reset counter
    }
    catch (const sdpa::client::ClientException &ce)
    {
      std::cout << "-" << std::flush;
      ++fail_count;
      continue;
    }

    if (sdpa::status::FINISHED == status)
    {
      break;
    }
    else if (sdpa::status::FAILED == status)
    {
      break;
    }
    else if (sdpa::status::CANCELED == status)
    {
      std::cerr << "cancelled!" << std::endl;
      break;
    }
    else
    {
      boost::this_thread::sleep(poll_interval);
    }
  }

  time_type poll_end = boost::posix_time::microsec_clock::local_time();

  std::cerr << "stopped at: " << poll_end << std::endl;
  std::cerr << "execution time: " << (poll_end - poll_start) << std::endl;
  return status;
}


int command_subscribe_and_wait ( const std::string &job_id
                               , const sdpa::client::ClientApi::ptr_t &ptrCli
                               , sdpa::client::job_info_t & job_info
                               )
{
  typedef boost::posix_time::ptime time_type;
  time_type poll_start = boost::posix_time::microsec_clock::local_time();

  std::cerr << "starting at: " << poll_start << std::endl;

  std::cerr << "waiting for job to return..." << std::flush;

  bool bSubscribed = false;
  int status = sdpa::status::UNKNOWN;
  int nTrials = 0;

  do
  {
	do
	{
		try
		{
			ptrCli->subscribe(job_id);
			bSubscribed = true;
		}
		catch(...)
		{
			bSubscribed = false;
			boost::this_thread::sleep(boost::posix_time::seconds(1));
		}

		if(bSubscribed)
			break;

		nTrials++;
		boost::this_thread::sleep(boost::posix_time::seconds(1));

	}while(nTrials<NMAXTRIALS);

	if(bSubscribed)
	{
		LOG(INFO, "The client successfully subscribed to the orchestrator for the job "<<job_id);
		nTrials = 0;
	}
	else
	{
		LOG(INFO, "Could not connect to the orchestrator. Giving-up, now!");
		return status;
	}

    try
    {
      seda::IEvent::Ptr reply( ptrCli->waitForNotification(0) );

      // check event type
      if (dynamic_cast<sdpa::events::JobFinishedEvent*>(reply.get()))
      {
        status = sdpa::status::FINISHED;
      }
      else if ( sdpa::events::JobFailedEvent *jfe
              = dynamic_cast<sdpa::events::JobFailedEvent*>(reply.get())
              )
      {
        status = sdpa::status::FAILED;
        job_info.error_code = jfe->error_code();
        job_info.error_message = jfe->error_message();
      }
      else if (dynamic_cast<sdpa::events::CancelJobAckEvent*>(reply.get()))
      {
        status = sdpa::status::CANCELED;
      }
      else if ( sdpa::events::ErrorEvent *err
              = dynamic_cast<sdpa::events::ErrorEvent*>(reply.get())
              )
      {
        std::cerr<< "got error event: reason := "
          + err->reason()
          + " code := "
          + boost::lexical_cast<std::string>(err->error_code())<<std::endl;
      }
      else
      {
        LOG(WARN, "unexpected reply: " << (reply ? reply->str() : "null"));
      }
    }
    catch (const sdpa::client::Timedout &)
    {
      LOG(WARN, "timeout expired!");
    }
  } while(status == sdpa::status::UNKNOWN);

  time_type poll_end = boost::posix_time::microsec_clock::local_time();

  std::cerr << "stopped at: " << poll_end << std::endl;
  std::cerr << "execution time: " << (poll_end - poll_start) << std::endl;

  return status;
}

/* returns the sdpa::status::code for the given job */
int command_wait ( const std::string &job_id
                 , const sdpa::client::ClientApi::ptr_t &api
                 , boost::posix_time::time_duration poll_interval
                 , sdpa::client::job_info_t & job_info
                 )
{
  if(poll_interval.total_milliseconds())
    return command_poll_and_wait(job_id, api, poll_interval, job_info);
  else
    return command_subscribe_and_wait(job_id, api, job_info);
}

bool file_exists(const std::string &path)
{
  struct stat file_info;
  int error_code = stat(path.c_str(), &file_info);
  if (error_code == 0)
        return true;
  else
        return false;
}

int main (int argc, char **argv) {
  const std::string name(argv[0]);
  namespace su = sdpa::util;

  std::string kvs_url (fhg::util::getenv("KVS_URL", "localhost:2439"));

  sdpa::client::config_t cfg = sdpa::client::ClientApi::config();
  cfg.tool_opts().add_options()
    ("output,o", su::po::value<std::string>(), "path to output file")
    ("wait,w", "wait until job is finished")
    ("poll-interval,t", su::po::value<int>()->default_value(100), "sets the poll interval")
    ("force,f", "force the operation")
    ("make-config", "create a basic config file")
    ("kvs,k", su::po::value<std::string>(&kvs_url)->default_value(kvs_url), "The kvs daemon's url")
    ("revision", "Dump the revision identifier")
    ("command", su::po::value<std::string>(),
     "The command that shall be performed. Possible values are:\n\n"
     "submit: \tsubmits a job to an orchestrator, arg must point to the job-description\n"
     "cancel: \tcancels a running job, arg must specify the job-id\n"
     "status: \tqueries the status of a job, arg must specify the job-id\n"
     "results: \tretrieve the results of a job, arg must specify the job-id\n"
     "delete: \tdelete a finished job, arg must specify the job-id\n"
     "wait: \twait until the job reaches a final state\n"
     )
    ;
  cfg.tool_hidden_opts().add_options()
    ("arg", su::po::value<std::vector<std::string> >(),
     "arguments to the command")
    ;
  cfg.positional_opts().add("command", 1).add("arg", -1);

  cfg.parse_command_line(argc, argv);
  cfg.parse_environment();

  cfg.notify();

  if (cfg.is_set("make-config"))
  {
    fs::path cfg_file (cfg.get("config"));
    if (fs::exists (cfg_file))
    {
      std::cerr << "E: config file '" << cfg_file << "' does already exist, please remove it first!" << std::endl;
      return FILE_EXISTS;
    }

    std::cout << "In order to create a configuration, I have to ask you some questions." << std::endl;
    std::cout << std::endl;

    std::string orchestrator_name(cfg.get<std::string>("orchestrator"));
    get_user_input("Name of the orchestrator ["+orchestrator_name+"]: ", orchestrator_name);

    std::string orchestrator_location("localhost:5000");
    get_user_input("Location of the orchestrator ["+orchestrator_location+"]: ", orchestrator_location);

    std::cout << std::endl;
    std::cout << "The information I gathered is:" << std::endl;

    std::stringstream sstr;

    sstr << "#" << std::endl;
    sstr << "# automatically generated sdpac config on " << boost::posix_time::second_clock::local_time() << std::endl;
    sstr << "#" << std::endl;
    sstr << "orchestrator = " << orchestrator_name << std::endl;
    sstr << std::endl;

    sstr << "[network]" << std::endl;
    sstr << "location = " << orchestrator_name << ":" << orchestrator_location << std::endl;
    sstr << std::endl;

    std::cout << sstr.str();

    std::cout << "Is that ok [y/N]? ";
    {
      std::string tmp;
      std::getline (std::cin, tmp);
      if (tmp != "y" && tmp != "Y")
        return 0;
    }

    // try to open config file
    fs::ofstream cfg_ofs(cfg_file);
    if ( ! cfg_ofs.is_open() )
    {
      fs::create_directories (cfg_file.parent_path());
      cfg_ofs.open (cfg_file);
      if ( ! cfg_ofs.is_open())
      {
        std::cerr << "E: could not open " << cfg_file << " for writing!" << std::endl;
        return IO_ERROR;
      }
    }

    cfg_ofs << sstr.str();

    std::cout << "Thank you, your config file has been written to " << cfg.get("config") << std::endl;
    return 0;
  }

  try
  {
    cfg.parse_config_file();
  }
  catch (std::exception const &)
  {
    // std::cerr << "W: could not parse config file: "
    //           << cfg.get("config")
    //           << std::endl;

    // std::cerr << "W: try generating one with '"
    //           << argv[0] << " --make-config'"
    //           << std::endl;
  }
  cfg.notify();

  if (cfg.is_set("help"))
  {
    cfg.printHelp(std::cout);
    return 0;
  }

  if (cfg.is_set("help-module"))
  {
    cfg.printModuleHelp(std::cout);
    return 0;
  }

  if (cfg.is_set("logging.file"))
  {
    try
    {
      fhg::log::getLogger().addAppender
        (fhg::log::Appender::ptr_t
        (new fhg::log::FileAppender( "logfile"
                                   , cfg.get<std::string>("logging.file")
                                   , "%t %s: %l %p:%L - %m%n"
                                   )
        )
        );
    }
    catch (const std::exception &ex)
    {
      std::cerr << "W: could not open logfile: " << cfg.get<std::string>("logging.file") << std::endl;
    }
  }

  if (cfg.is_set("logging.tostderr"))
  {
    fhg::log::getLogger().addAppender
      (fhg::log::Appender::ptr_t(new fhg::log::StreamAppender( "console"
                                                             , std::cerr
                                                             , "%s: %p:%L - %m%n"
                                                             )
                                )
      );
  }
  if (cfg.is_set("quiet"))
  {
    fhg::log::getLogger().setLevel(fhg::log::LogLevel::ERROR);
  }
  else
  {
    fhg::log::getLogger().setLevel(fhg::log::LogLevel::WARN);
  }

  if (cfg.is_set("verbose"))
  {
    int lvl(cfg.get<int>("verbose"));
    if (lvl > 0) fhg::log::getLogger().setLevel(fhg::log::LogLevel::INFO);
    if (lvl > 1) fhg::log::getLogger().setLevel(fhg::log::LogLevel::DEBUG);
    if (lvl > 2) fhg::log::getLogger().setLevel(fhg::log::LogLevel::TRACE);
  }

  try
  {
    std::string client_api_name ("sdpac-");
    {
      client_api_name +=
        boost::uuids::to_string (boost::uuids::random_generator()());
    }

    sdpa::client::ClientApi::ptr_t api
      (sdpa::client::ClientApi::create (cfg, client_api_name));
    if (cfg.is_set("version"))
    {
      std::cerr << fhg::project_info ("GPI-Space Client");
      return 0;
    }
    if (cfg.is_set("dumpversion"))
    {
      std::cout << fhg::project_version() << std::endl;
      return 0;
    }
    if (cfg.is_set("revision"))
    {
      std::cout << fhg::project_revision() << std::endl;
      return 0;
    }

    try
    {
      // initialize the KVS

      LOG(INFO, "initializing KVS at " << kvs_url);

      std::vector<std::string> parts;
      fhg::log::split(kvs_url, ":", std::back_inserter(parts));
      if (parts.size() != 2)
      {
        LOG(ERROR, "invalid kvs url: expected host:port, got: " << kvs_url);
        return EXIT_FAILURE;
      }
      else
      {
        fhg::com::kvs::global::get_kvs_info().init( parts[0]
                                                  , parts[1]
                                                  , boost::posix_time::seconds(120)
                                                  , 1
                                                  );
      }
    }
    catch (std::exception const & ex)
    {
      std::cerr << "E: could not connect to KVS: " << ex.what() << std::endl;
      return EXIT_FAILURE;
    }

    if (! cfg.is_set("command"))
    {
        std::cerr << "E: a command is required!" << std::endl;
        std::cerr << "E: type --help to get a list of available options!" << std::endl;
        return ERR_USAGE;
    }

    const std::string &command(cfg.get("command"));

    std::vector<std::string> args;
    if (cfg.is_set("arg"))
    {
      args = cfg.get<std::vector<std::string> >("arg");
    }

    LOG(INFO, "***************************************************");
    LOG(INFO, fhg::project_summary() << " (" << fhg::project_version() << ")");
    LOG(INFO, "***************************************************");

    try
    {
      api->configure_network(cfg);
    }
    catch (const std::exception &ex)
    {
      std::cerr << "E: network connection could not be set up: " << ex.what() << std::endl;
      return NETWORK_ERROR;
    }

    if (command == "submit")
    {
      if (args.empty())
      {
        std::cerr << "E: path required" << std::endl;
        return ERR_USAGE;
      }
      std::ifstream ifs(args.front().c_str());
      if (! ifs.good())
      {
        std::cerr << "could not open: " << args.front() << std::endl;
        return IO_ERROR;
      }

      std::stringstream sstr;
      ifs >> std::noskipws >> sstr.rdbuf();

      const std::string job_id(api->submitJob(sstr.str()));
      std::cout << job_id << std::endl;

      if (cfg.is_set("wait"))
      {
        const int poll_interval = cfg.get<int>("poll-interval");
        sdpa::client::job_info_t job_info;
        int status = command_wait( job_id
                                    , api
                                    , boost::posix_time::milliseconds(poll_interval)
                                    , job_info
                                    );
        if (sdpa::status::FAILED == status)
        {
          std::cerr << "failed: "
                    << "error-code"
                    << " := "
                    << fhg::error::show(job_info.error_code)
                    << " (" << job_info.error_code << ")"
                    << std::endl
                    << "error-message := " << job_info.error_message
                    << std::endl
            ;
        }

        std::cerr << "retrieve the results with:" << std::endl;
        std::cerr << "\t" << argv[0] << " results " << job_id << std::endl;
        std::cerr << "delete the job with:" << std::endl;
        std::cerr << "\t" << argv[0] << " delete " << job_id << std::endl;

        return status;
      }
    }
    else if (command == "wait")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required" << std::endl;
        return JOB_ID_MISSING;
      }

      const std::string job_id (args.front());
      const int poll_interval = cfg.get<int>("poll-interval");
      sdpa::client::job_info_t job_info;

      int status = command_wait( job_id
                               , api
                               , boost::posix_time::milliseconds(poll_interval)
                               , job_info
                               );
      if (sdpa::status::FAILED == status)
      {
        std::cerr << "failed: "
                  << "error-code"
                  << " := "
                  << fhg::error::show(job_info.error_code)
                  << " (" << job_info.error_code << ")"
                  << std::endl
                  << "error-message := " << job_info.error_message
                  << std::endl
          ;
      }

      std::cerr << "retrieve the results with:" << std::endl;
      std::cerr << "\t" << argv[0] << " results " << job_id << std::endl;
      std::cerr << "delete the job with:" << std::endl;
      std::cerr << "\t" << argv[0] << " delete " << job_id << std::endl;

      return status;
    }
    else if (command == "cancel")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required" << std::endl;
        return JOB_ID_MISSING;
      }
      api->cancelJob(args.front());
    }
    else if (command == "status")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required" << std::endl;
        return JOB_ID_MISSING;
      }
      sdpa::client::job_info_t job_info;
      int status = api->queryJob(args.front(), job_info);
      std::cout << sdpa::status::show(status) << std::endl;
      if (status == sdpa::status::FAILED)
      {
        std::cerr << "error-code"
                  << " := "
                  << fhg::error::show(job_info.error_code)
                  << " (" << job_info.error_code << ")"
                  << std::endl
                  << "error-message := " << job_info.error_message
                  << std::endl
          ;
      }

      return status;
    }
    else if (command == "results")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required" << std::endl;
        return JOB_ID_MISSING;
      }

      const std::string job_id(args.front());
      std::string output_path
        (cfg.is_set ("output") ? cfg.get("output") : ("sdpa." + job_id + ".out"));

          if (file_exists(output_path) && (! cfg.is_set("force")))
          {
                std::cerr << "E: output-file " << output_path << " does already exist!" << std::endl;
                return FILE_EXISTS;
          }

      std::ofstream ofs(output_path.c_str());
          if (! ofs)
          {
                std::cerr << "E: could not open " << output_path << " for writing!" << std::endl;
                return IO_ERROR;
          }

      sdpa::client::result_t results(api->retrieveResults(job_id));
      ofs << results << std::flush;
      std::cerr << "stored results in: " << output_path << std::endl;
    }
    else if (command == "delete")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required" << std::endl;
        return JOB_ID_MISSING;
      }
      api->deleteJob(args.front());
    }
    else if (command == "wait")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required!" << std::endl;
        return JOB_ID_MISSING;
      }
      const std::string &job_id(args.front());
      const int poll_interval = cfg.get<int>("wait");
      sdpa::client::job_info_t job_info;
      int status = command_wait( job_id
                               , api
                               , boost::posix_time::milliseconds(poll_interval)
                               , job_info
                               );

      if (status == sdpa::status::FAILED)
      {
        std::cerr << "error-code"
                  << " := "
                  << fhg::error::show(job_info.error_code)
                  << " (" << job_info.error_code << ")"
                  << std::endl
                  << "error-message := " << job_info.error_message
                  << std::endl
          ;
      }

      std::cerr << "retrieve the results with:" << std::endl;
      std::cerr << "\t" << argv[0] << " results " << job_id << std::endl;
      std::cerr << "delete the job with:" << std::endl;
      std::cerr << "\t" << argv[0] << " delete " << job_id << std::endl;

      return status;
    }
    else
    {
      std::cerr << "illegal command: " << command << std::endl;
      return (ERR_USAGE);
    }

    api->shutdown_network();
  }
  catch (std::exception const &ex)
  {
    std::cerr << "sdpac: failed: " << ex.what() << std::endl;
    return UNKNOWN_ERROR;
  }
}
