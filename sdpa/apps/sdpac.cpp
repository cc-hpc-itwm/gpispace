#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
//#include <unistd.h>
#include <sys/stat.h> // for stat, to check if file exists

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>
#include <fhglog/FileAppender.hpp>
#include <fhglog/StreamAppender.hpp>

#include <fhg/util/getenv.hpp>

#include <sdpa/job_states.hpp>
#include <sdpa/client.hpp>
#include <seda/IEvent.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include <fhgcom/kvs/kvsc.hpp>

#include <fhg/error_codes.hpp>
#include <fhg/revision.hpp>
#include <fhg/util/read_bool.hpp>

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

void get_user_input(std::string const & prompt, std::string & result, std::istream & in = std::cin)
{
  std::cout << prompt;
  std::string tmp;
  std::getline (in, tmp);
  if (tmp.size())
    result = tmp;
}

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
    po::options_description& network_opts() { return network_opts_;} // network related options

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
    po::options_description network_opts_; // network related options
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
    , network_opts_("Network Options")
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
    network_opts_.add_options()
      ("network.timeout", po::value<unsigned int>(),
      "maximum time to wait for a reply (in milliseconds)")
      ;
    specific_opts_.add_options()
      ( "orchestrator"
      , po::value<std::string>()->default_value ("orchestrator")
      , "name of the orchestrator"
      )
      ( "config,C"
      , po::value<std::string>()->default_value
      (std::getenv ("HOME") + std::string ("/.sdpa/configs/sdpac.rc"))
      , "path to the configuration file"
      );
  }

  void NewConfig::parse_command_line(int argc, char **argv)
  {
    po::options_description desc;
    desc.add(generic_opts_)
      .add(logging_opts_)
      .add(network_opts_)
      .add(specific_opts_)
      .add(tool_opts_)
      .add(tool_hidden_opts_);

    po::store(po::command_line_parser(argc, argv).options(desc)
             .positional(positional_opts_)
             .run()
             , opts_);
  }

  void NewConfig::parse_config_file()
  {
    if (is_set("config"))
    {
      const std::string &cfg_file = get("config");
      if (is_set("verbose"))
      {
        std::cerr << "I: using config file: " << cfg_file << std::endl;
      }
      std::ifstream stream(cfg_file.c_str());
      if (! stream)
      {
        throw std::runtime_error ("could not read config file: " + cfg_file);
      }
      else
      {
        po::options_description desc;
        desc.add(logging_opts_)
          .add(network_opts_)
          .add(specific_opts_);
        po::store(po::parse_config_file(stream, desc), opts_);
      }
    }
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
      .add(network_opts_)
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

    if      (mod == "network")       os << network_opts_;
    else if (mod == "logging")       os << logging_opts_;
    else if (mod == component_name_) os << specific_opts_;
    else
    {
      os << "Available modules are:" << std::endl
         << "\t" << "network" << std::endl
         << "\t" << "logging" << std::endl
         << "\t" << component_name_ << std::endl
         << "use --help-module=module to get more information" << std::endl;
    }
    return os;
  }
}

int main (int argc, char **argv) {
  const std::string name(argv[0]);

  std::string kvs_url (fhg::util::getenv("KVS_URL", "localhost:2439"));

  NewConfig cfg;
  cfg.tool_opts().add_options()
    ("output,o", po::value<std::string>(), "path to output file")
    ("wait,w", "wait until job is finished")
    ("polling", po::value<std::string>()->default_value ("true"), "use polling when waiting for job completion")
    ("force,f", "force the operation")
    ("kvs,k", po::value<std::string>(&kvs_url)->default_value(kvs_url), "The kvs daemon's url")
    ("revision", "Dump the revision identifier")
    ("command", po::value<std::string>(),
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
    ("arg", po::value<std::vector<std::string> >(),
     "arguments to the command")
    ;
  cfg.positional_opts().add("command", 1).add("arg", -1);

  cfg.parse_command_line(argc, argv);
  cfg.parse_environment();

  cfg.notify();

  try
  {
    cfg.parse_config_file();
  }
  catch (std::exception const &)
  {
    // std::cerr << "W: could not parse config file: "
    //           << cfg.get("config")
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

    sdpa::client::Client api ( cfg.is_set("orchestrator")
                             ? cfg.get<std::string>("orchestrator")
                             : throw std::runtime_error ("no orchestrator specified!")
                             , cfg.is_set("network.timeout")
                             ? boost::optional<boost::posix_time::time_duration>
                               ( cfg.get<boost::posix_time::milliseconds>
                                 ("network.timeout")
                               )
                             : boost::none
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
    else
    {
      std::cerr << "illegal command: " << command << std::endl;
      return (ERR_USAGE);
    }
  }
  catch (std::exception const &ex)
  {
    std::cerr << "sdpac: failed: " << ex.what() << std::endl;
    return UNKNOWN_ERROR;
  }
}
