#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
//#include <unistd.h>
#include <sys/stat.h> // for stat, to check if file exists

#include <fhglog/LogMacros.hpp>
#include <fhglog/Configuration.hpp>
#include <fhglog/appender/file.hpp>
#include <fhglog/appender/stream.hpp>

#include <fhg/util/getenv.hpp>
#include <fhg/util/split.hpp>

#include <sdpa/job_states.hpp>
#include <sdpa/client.hpp>

#include <we/type/value/read.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include <fhgcom/kvs/kvsc.hpp>

#include <fhg/revision.hpp>
#include <fhg/util/read_bool.hpp>

#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/id_generator.hpp>

namespace fs = boost::filesystem;

enum return_codes_t
  {
    ERR_USAGE             = 50
  , JOB_ID_MISSING        = 51
  , FILE_EXISTS           = 52
  , IO_ERROR              = 53
  , UNKNOWN_ERROR         = 100
  };

namespace
{
  sdpa::status::code wait_for_terminal_state ( sdpa::client::Client& api
                                             , const std::string job_id
                                             , const bool do_polling
                                             , const std::string app_name
                                             )
  {
    const boost::posix_time::ptime poll_start
      (boost::posix_time::microsec_clock::local_time());

    std::cerr << "starting at: " << poll_start << std::endl;
    std::cerr << "waiting for job to return..." << std::flush;

    sdpa::client::job_info_t job_info;
    const sdpa::status::code status
      ( do_polling
      ? api.wait_for_terminal_state_polling (job_id, job_info)
      : api.wait_for_terminal_state (job_id, job_info)
      );

    const boost::posix_time::ptime poll_end
      (boost::posix_time::microsec_clock::local_time());

    std::cerr << "stopped at: " << poll_end << std::endl;
    std::cerr << "execution time: " << (poll_end - poll_start) << std::endl;

    if (sdpa::status::FAILED == status)
    {
      std::cerr << "failed: "
                << "error-message := " << job_info.error_message
                << std::endl
        ;
    }

    std::cerr << "retrieve the results with:" << std::endl;
    std::cerr << "\t" << app_name << " results " << job_id << std::endl;
    std::cerr << "delete the job with:" << std::endl;
    std::cerr << "\t" << app_name << " delete " << job_id << std::endl;

    return status;
  }

  namespace po = boost::program_options;

  class NewConfig
  {
  public:
    NewConfig();

    void parse_command_line(int argc, char **argv);

    void parse_config_file();

    void parse_environment();

    void notify(); // notify that parsing is finished

    po::options_description &specific_opts() { return specific_opts_; }
    po::options_description &tool_opts() { return tool_opts_; }
    po::options_description &tool_hidden_opts() { return tool_hidden_opts_; }
    po::positional_options_description &positional_opts() { return positional_opts_; }

    std::ostream &printHelp(std::ostream &) const;
    std::ostream &printModuleHelp(std::ostream &os) const;

    const std::string &get(const std::string &name) const { return opts_[name].as<std::string>(); }
    template <typename T> const T &get(const std::string &name) const { return opts_[name].as<T>(); }
    template <typename T> const T &operator[](const std::string &name) const { return get<T>(name); }

    bool is_set(const std::string &name) const { return (opts_.count(name) > 0); }
  private:
    std::string component_name_;
    std::string env_prefix_;

    po::options_description generic_opts_; // supported by all command line tools
    po::options_description logging_opts_; // logging related options
    po::options_description tool_opts_;    // additional command line options
    po::options_description tool_hidden_opts_; // hidden command line options
    po::options_description specific_opts_; // specific options to a component, feel free to modify them
    po::positional_options_description positional_opts_; // positional parameters used for command line parsing

    po::variables_map opts_;
  };

  NewConfig::NewConfig()
    : component_name_("client")
    , env_prefix_("SDPAC_")
    , generic_opts_("Generic Options")
    , logging_opts_("Logging configuration")
    , tool_opts_("Command-line tool options")
    , tool_hidden_opts_("Command-line tool options (hidden)")
    , specific_opts_(component_name_ + " specific options")
  {
    // fill in defaults
    generic_opts_.add_options()
      ("help,h", "show this help text")
      ("help-module", po::value<std::string>()->implicit_value("help"),
      "show the help for a specific module")
      ("version,V", "print the version number")
      ("dumpversion", "print the version number (short)")
      ("verbose,v", po::value<int>()->implicit_value(1),
      "verbosity level")
      ("quiet,q", "be quiet")
      ;
    logging_opts_.add_options()
      ("logging.file", po::value<std::string>(),
      "redirect log output to this file")
      ("logging.tostderr", "output to stderr")
      ;
    specific_opts_.add_options()
      ( "orchestrator-host"
      , po::value<std::string>()->required()
      , "host of the orchestrator"
      )
      ( "orchestrator-port"
      , po::value<unsigned short>()->required()
      , "port of the orchestrator"
      );
  }

  void NewConfig::parse_command_line(int argc, char **argv)
  {
    po::options_description desc;
    desc.add(generic_opts_)
      .add(logging_opts_)
      .add(specific_opts_)
      .add(tool_opts_)
      .add(tool_hidden_opts_);

    po::store(po::command_line_parser(argc, argv).options(desc)
             .positional(positional_opts_)
             .run()
             , opts_);
  }

  struct environment_variable_to_option
  {
  public:
    explicit
    environment_variable_to_option(const std::string prefix)
      : p(prefix) {}

    std::string operator()(const std::string &var)
    {
      if (var.substr(0, p.size()) == p)
      {
        std::string option = var.substr(p.size());
        std::transform(option.begin(), option.end(), option.begin(), tolower);
        for (std::string::iterator c(option.begin()); c != option.end(); ++c)
        {
          if (*c == '_') *c = '.';
        }
        return option;
      }
      return "";
    }

  private:
    std::string p;
  };

  void NewConfig::parse_environment()
  {
    po::options_description desc;
    desc.add(logging_opts_)
      .add(specific_opts_);
    po::store(po::parse_environment(desc, environment_variable_to_option(env_prefix_)) , opts_);
  }

  void NewConfig::notify()
  {
    po::notify(opts_);
  }

  std::ostream &NewConfig::printHelp(std::ostream &os) const
  {
    po::options_description desc("Allowed Options");
    desc.add(generic_opts_).add(tool_opts_);
    os << desc;
    return os;
  }

  std::ostream &NewConfig::printModuleHelp(std::ostream &os) const
  {
    const std::string mod (get<std::string>("help-module"));

    if      (mod == "logging")       os << logging_opts_;
    else if (mod == component_name_) os << specific_opts_;
    else
    {
      os << "Available modules are:" << std::endl
         << "\t" << "logging" << std::endl
         << "\t" << component_name_ << std::endl
         << "use --help-module=module to get more information" << std::endl;
    }
    return os;
  }
}

int main (int argc, char **argv) {
  const std::string name(argv[0]);

  NewConfig cfg;
  cfg.tool_opts().add_options()
    ("output,o", po::value<std::string>(), "path to output file")
    ("wait,w", "wait until job is finished")
    ("polling", po::value<std::string>()->default_value ("true"), "use polling when waiting for job completion")
    ("force,f", "force the operation")
    ("kvs-host", po::value<std::string>()->required(), "The kvs daemon's host")
    ("kvs-port", po::value<std::string>()->required(), "The kvs daemon's port")
    ("revision", "Dump the revision identifier")
    ("command", po::value<std::string>()->required(),
     "The command that shall be performed. Possible values are:\n\n"
     "submit: \tsubmits a job to an orchestrator, arg must point to the job-description\n"
     "cancel: \tcancels a running job, arg must specify the job-id\n"
     "status: \tqueries the status of a job, arg must specify the job-id\n"
     "results: \tretrieve the results of a job, arg must specify the job-id\n"
     "delete: \tdelete a finished job, arg must specify the job-id\n"
     "wait: \twait until the job reaches a final state\n"
     "discover: \tdiscover the states of the known jobs\n"
     "put_token: \tput token onto place in job\n"
     )
    ;
  cfg.tool_hidden_opts().add_options()
    ("arg", po::value<std::vector<std::string>>(),
     "arguments to the command")
    ;
  cfg.positional_opts().add("command", 1).add("arg", -1);

  cfg.parse_command_line(argc, argv);
  cfg.parse_environment();

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

  cfg.notify();

  if (cfg.is_set("logging.file"))
  {
    try
    {
      fhg::log::Logger::get()->addAppender
        (fhg::log::Appender::ptr_t
        (new fhg::log::FileAppender( cfg.get<std::string>("logging.file")
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
    fhg::log::Logger::get()->addAppender
      (fhg::log::Appender::ptr_t(new fhg::log::StreamAppender( std::cerr
                                                             , "%s: %p:%L - %m%n"
                                                             )
                                )
      );
  }
  if (cfg.is_set("quiet"))
  {
    fhg::log::Logger::get()->setLevel(fhg::log::ERROR);
  }
  else
  {
    fhg::log::Logger::get()->setLevel(fhg::log::WARN);
  }

  if (cfg.is_set("verbose"))
  {
    int lvl(cfg.get<int>("verbose"));
    if (lvl > 0) fhg::log::Logger::get()->setLevel(fhg::log::INFO);
    if (lvl > 1) fhg::log::Logger::get()->setLevel(fhg::log::TRACE);
  }

  try
  {
    fhg::log::Logger::ptr_t logger (fhg::log::Logger::get ("sdpac"));

    const std::string &command(cfg.get("command"));

    std::vector<std::string> args;
    if (cfg.is_set("arg"))
    {
      args = cfg.get<std::vector<std::string>>("arg");
    }

    LLOG (INFO, logger, "***************************************************");
    LLOG (INFO, logger, fhg::project_summary() << " (" << fhg::project_version() << ")");
    LLOG (INFO, logger, "***************************************************");

    boost::asio::io_service peer_io_service;
    boost::asio::io_service kvs_client_io_service;
    sdpa::client::Client api
      ( fhg::com::host_t (cfg.get<std::string>("orchestrator-host"))
      , fhg::com::port_t
          (std::to_string (cfg.get<unsigned short>("orchestrator-port")))
      , peer_io_service
      , kvs_client_io_service
      , cfg.get ("kvs-host"), cfg.get ("kvs-port")
      );

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

      const std::string job_id(api.submitJob(sstr.str()));
      std::cout << job_id << std::endl;

      if (cfg.is_set("wait"))
      {
        return wait_for_terminal_state (api, job_id, fhg::util::read_bool (cfg.get<std::string> ("polling")), argv[0]);
      }
    }
    else if (command == "wait")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required" << std::endl;
        return JOB_ID_MISSING;
      }

      return wait_for_terminal_state (api, args.front(), fhg::util::read_bool (cfg.get<std::string> ("polling")), argv[0]);
    }
    else if (command == "cancel")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required" << std::endl;
        return JOB_ID_MISSING;
      }
      api.cancelJob(args.front());
    }
    else if (command == "status")
    {
      if (args.empty())
      {
        std::cerr << "E: job-id required" << std::endl;
        return JOB_ID_MISSING;
      }
      sdpa::client::job_info_t job_info;
      const sdpa::status::code status (api.queryJob(args.front(), job_info));
      std::cout << sdpa::status::show(status) << std::endl;
      if (status == sdpa::status::FAILED)
      {
        std::cerr << "error-message := " << job_info.error_message
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

          if (fs::exists(output_path) && (! cfg.is_set("force")))
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

      sdpa::client::result_t results(api.retrieveResults(job_id));
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
      api.deleteJob(args.front());
    }
    else if (command == "discover")
    {
      if (args.empty())
      {
          std::cerr << "E: job-id required" << std::endl;
          return JOB_ID_MISSING;
      }

      const we::layer::id_type discover_id (sdpa::id_generator ("discover_id").next());
      const std::string job_id (args.front());
      sdpa::discovery_info_t discovery_result (api.discoverJobStates(discover_id, job_id));
      std::cout<<"discovery result: "<<discovery_result<<std::endl;
    }
    else if (command == "put_token")
    {
      if (args.size() == 3)
      {
        throw std::invalid_argument
          ("put_token requires <job_id> <place_name> <value>");
      }

      sdpa::job_id_t const job_id (args.at (0));
      std::string const place_name (args.at (1));
      std::string const value (args.at (2));

      api.put_token (job_id, place_name, pnet::type::value::read (value));
    }
    else
    {
      std::cerr << "unknown command: " << command << std::endl;
      return (ERR_USAGE);
    }
  }
  catch (std::exception const &ex)
  {
    std::cerr << "sdpac: failed: " << ex.what() << std::endl;
    return UNKNOWN_ERROR;
  }
}
