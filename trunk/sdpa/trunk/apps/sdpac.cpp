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

#include <sdpa/client/ClientApi.hpp>
#include <sdpa/util/util.hpp>
#include <sdpa/util/Config.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/tokenizer.hpp>
#include <fhgcom/kvs/kvsc.hpp>


namespace fs = boost::filesystem;

void get_user_input(std::string const & prompt, std::string & result, std::istream & in = std::cin)
{
  std::cout << prompt;
  std::string tmp;
  std::getline (in, tmp);
  if (tmp.size())
    result = tmp;
}

static std::string center (std::string const & text, const std::size_t len)
{
  if (len >= text.size())
  {
    std::size_t diff (len - text.size());
    std::string indent; indent.resize( diff / 2, ' ');
    return indent + text;
  }
  else
  {
    return text;
  }
}

/* returns: 0 job finished, 1 job failed, 2 job cancelled, other value if failures occurred */
int command_wait(const std::string &job_id, const sdpa::client::ClientApi::ptr_t &api, int poll_interval)
{
  boost::system_time poll_start = boost::get_system_time();
  std::cerr << "starting at: " << poll_start << std::endl;

  std::cout << "waiting for job to return..." << std::flush;

  int exit_code(4);
  std::size_t fail_count(0);
  for (; fail_count < 3 ;)
  {
    std::string status;
    try
    {
      status = api->queryJob(job_id);
      fail_count = 0; // reset counter
    }
    catch (const sdpa::client::ClientException &ce)
    {
      std::cout << "-" << std::flush;
      ++fail_count;
      continue;
    }

    if (status == "SDPA::Finished")
    {
      std::cout << "finished!" << std::endl;
      exit_code = 0;
      break;
    }
    else if (status == "SDPA::Failed")
    {
      std::cout << "failed!" << std::endl;
      exit_code = 1;
      break;
    }
    else if (status == "SDPA::Cancelled")
    {
      std::cout << "cancelled!" << std::endl;
      exit_code = 2;
      break;
    }
    else
    {
	boost::this_thread::sleep(boost::posix_time::seconds(poll_interval));
	std::cout << "." << std::flush;
    }
  }
  boost::system_time poll_end = boost::get_system_time();
  boost::posix_time::time_period tp(poll_start, poll_end);

  std::cerr << "stopped at: " << poll_end << std::endl;
  std::cerr << "execution time: " << tp.length() << std::endl;
  return exit_code;
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
    ("poll-interval,t", su::po::value<int>()->default_value(1), "sets the poll interval")
    ("force,f", "force the operation")
    ("make-config", "create a basic config file")
    ("kvs,k", su::po::value<std::string>(&kvs_url)->default_value(kvs_url), "The kvs daemon's url")
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
      return 1;
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
        return 1;
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
        return 1;
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
    std::cerr << "could not parse config file: " << cfg.get("config") << std::endl;
    std::cerr << "try generating one with '" << argv[0] << " --make-config'"  << std::endl;
    return 1;
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
    sdpa::client::ClientApi::ptr_t api(sdpa::client::ClientApi::create(cfg));
    if (cfg.is_set("version"))
    {
      const std::size_t maxlen (72);
      const std::string header ("SDPA - Seismic Data Processing Architecture");
      std::string seperator; seperator.resize (header.size(), '=');

      std::vector <std::string> lines;
      lines.push_back (header);
      lines.push_back (seperator);
      lines.push_back ("");
      lines.push_back (api->revision());
      lines.push_back (api->copyright());
      lines.push_back (api->build_timestamp());
      lines.push_back (api->contact());

      for ( std::vector<std::string>::iterator line (lines.begin())
	  ; line != lines.end()
	  ; ++line
	  )
      {
	if (! line->empty())
	{
	  std::cout << center (*line, maxlen);
	}
	std::cout << std::endl;
      }
      return 0;
    }
    if (cfg.is_set("dumpversion"))
    {
      std::cout << api->revision() << std::endl;
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
                                                  , boost::posix_time::seconds(1)
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
    	return 1;
    }

    const std::string &command(cfg.get("command"));

    std::vector<std::string> args;
    if (cfg.is_set("arg"))
    {
      args = cfg.get<std::vector<std::string> >("arg");
    }

    LOG(INFO, "***************************************************");
    LOG(INFO, "SDPA - Seismic Data Processing Architecture (" << api->version() << ")");
    LOG(INFO, "***************************************************");

    try
    {
      api->configure_network(cfg);
    }
    catch (const std::exception &ex)
    {
      std::cerr << "E: network connection could not be set up: " << ex.what() << std::endl;
      return 2;
    }

    if (command == "submit")
    {
      if (args.empty())
      {
        std::cerr << "E: path required" << std::endl;
        return 2;
      }
      std::ifstream ifs(args.front().c_str());
      if (! ifs.good())
      {
        std::cerr << "could not open: " << args.front() << std::endl;
        return (2);
      }

      std::stringstream sstr;
      ifs >> sstr.rdbuf();

      const std::string job_id(api->submitJob(sstr.str()));
      std::cout << job_id << std::endl;

      if (cfg.is_set("wait"))
      {
        const int poll_interval = cfg.get<int>("poll-interval");
        int wait_code = command_wait(job_id, api, poll_interval);

        switch (wait_code)
        {
          case 0: // finished
          case 1: // failed
          case 2: // cancelled
          {
            std::cerr << "retrieve the results with:" << std::endl;
            std::cout << "\t" << argv[0] << " results " << job_id << std::endl;
            std::cerr << "delete the job with:" << std::endl;
            std::cout << "\t" << argv[0] << " delete " << job_id << std::endl;
            break;
          }
          default:
            std::cerr << "could not get status information!" << std::endl;
            break;
        }
        return wait_code;
      }
    }
    else if (command == "wait")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required" << std::endl;
        return 4;
      }

      const std::string job_id (args.front());
      const int poll_interval = cfg.get<int>("poll-interval");
      int wait_code = command_wait(job_id, api, poll_interval);

      switch (wait_code)
      {
      case 0: // finished
      case 1: // failed
      case 2: // cancelled
        {
          std::cerr << "retrieve the results with:" << std::endl;
          std::cerr << "\t" << argv[0] << " results " << job_id << std::endl;
          std::cerr << "delete the job with:" << std::endl;
          std::cerr << "\t" << argv[0] << " delete " << job_id << std::endl;
          break;
        }
      default:
        std::cerr << "could not get status information!" << std::endl;
        break;
      }

      return wait_code;
    }
    else if (command == "cancel")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required" << std::endl;
        return 4;
      }
      api->cancelJob(args.front());
    }
    else if (command == "status")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required" << std::endl;
        return 4;
      }
      std::cout << api->queryJob(args.front()) << std::endl;
    }
    else if (command == "results")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required" << std::endl;
        return 4;
      }

      const std::string job_id(args.front());
      std::string output_path
        (cfg.is_set ("output") ? cfg.get("output") : ("sdpa." + job_id + ".out"));

	  if (file_exists(output_path) && (! cfg.is_set("force")))
	  {
		std::cerr << "E: output-file " << output_path << " does already exist!" << std::endl;
		return 4;
	  }

      std::ofstream ofs(output_path.c_str());
	  if (! ofs)
	  {
		std::cerr << "E: could not open " << output_path << " for writing!" << std::endl;
		return 4;
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
        return 4;
      }
      api->deleteJob(args.front());
    }
    else if (command == "wait")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required!" << std::endl;
        return 4;
      }
      const std::string &job_id(args.front());
      const int poll_interval = cfg.get<int>("wait");
      int wait_code = command_wait(job_id, api, poll_interval);

      switch (wait_code)
      {
        case 0: // finished
        case 1: // failed
        case 2: // cancelled
        {
          std::cerr << "retrieve the results with:" << std::endl;
          std::cout << "\t" << argv[0] << " results " << job_id << std::endl;
          std::cerr << "delete the job with:" << std::endl;
          std::cout << "\t" << argv[0] << " delete " << job_id << std::endl;
          break;
        }
        default:
          std::cerr << "could not get status information!" << std::endl;
          break;
      }
      return wait_code;
    }
    else
    {
      std::cerr << "illegal command: " << command << std::endl;
      return (1);
    }

    api->shutdown_network();
  }
  catch (const sdpa::client::ClientException &ce)
  {
    std::cerr << "failed: " << ce.what() << std::endl;
    return 3;
  }
}
