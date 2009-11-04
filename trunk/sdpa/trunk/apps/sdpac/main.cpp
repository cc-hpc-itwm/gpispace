#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unistd.h>

#include <sdpa/sdpa-config.hpp>

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>

#include <sdpa/client/ClientApi.hpp>

#include <boost/program_options.hpp>
#include <algorithm> // std::transform
#include <cctype> // std::tolower

struct environment_variable_to_option
{
  environment_variable_to_option(const std::string prefix="SDPAC_")
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

  std::string p;
};

int main (int argc, char **argv) {
  const std::string name(argv[0]);
  namespace po = boost::program_options;

  po::options_description general_opts("General options");
  general_opts.add_options()
    ("help,h", "show this help text")
    ("help-module", po::value<std::string>()->implicit_value("help"),
     "show the help for a specific module")
    ("version,V", "print the version number")
    ("dumpversion", "print the version number (short)")
    ;
  
  po::options_description logging_opts("Logging related options");
  logging_opts.add_options()
    ("logging.file", po::value<std::string>()->default_value("sdpac.log"),
     "redirect log output to this file")
    ("logging.tostderr", "output to stderr")
    ("logging.level", po::value<int>()->default_value(3),
     "standard logging level")
     ;

  po::options_description client_opts("Client specific options");
  client_opts.add_options()
    ("command,c", po::value<std::string>(),
     "The command that shall be performed. Possible values are:\n\n"
     "submit: \tsubmits a job to an orchestrator, arg must point to the job-description\n"
     "cancel: \tcancels a running job, arg must specify the job-id\n"
     "status: \tqueries the status of a job, arg must specify the job-id\n"
     "results: \tretrieve the results of a job, arg must specify the job-id\n"
     "delete: \tdelete a finished job, arg must specify the job-id\n"
     )
    ("output", po::value<std::string>()->default_value("sdpac.out"),
     "path to output file")
    ("orchestrator", po::value<std::string>()->default_value("orchestrator"),
     "name of the orchestrator")
    ("client", po::value<std::string>()->default_value("sdpa.app.client"),
     "name of the client")
    ("config,C", po::value<std::string>()->default_value(std::string(SDPA_PREFIX) + "/etc/sdpac.rc"),
     "path to the configuration file")
    ("verbose,v", po::value<int>()->implicit_value(1),
     "verbosity level")
    ("quiet,q", "be quiet")
    ;

  po::options_description client_hidden;
  client_hidden.add_options()
    ("arg", po::value<std::vector<std::string> >(),
     "arguments to the command")
     ;

  po::options_description network_opts("Network related options");
  network_opts.add_options()
    ("network.timeout", po::value<unsigned int>()->default_value(30000),
     "maximum time to wait for a reply (in milliseconds)")
    ("network.location", po::value< std::vector<std::string> >()->composing(),
     "location information for a specific location (name:location)")
    ;

  po::options_description visible_opts("Allowed options");
  visible_opts.add(general_opts).add(client_opts);

  po::options_description cmdline_opts;
  cmdline_opts.add(visible_opts).add(client_hidden).add(network_opts).add(logging_opts);

  po::options_description config_opts;
  config_opts.add(client_opts).add(network_opts).add(logging_opts);
  
  po::positional_options_description p;
  p.add("command", 1);
  p.add("arg", -1);
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).
            options(cmdline_opts).positional(p).run(), vm);
  po::store(po::parse_environment(config_opts, environment_variable_to_option("SDPAC_")) , vm);
  {
    if (vm.count("config"))
    {
      std::ifstream cfg_s(vm["config"].as<std::string>().c_str());
      if (! cfg_s)
      {
        std::cerr << "W: could not open " << vm["config"].as<std::string>() << " for reading" << std::endl;
      }
      else
      {
        po::store(po::parse_config_file(cfg_s, config_opts), vm);
      }
    }
  }
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cout << visible_opts;
    return 0;
  }

  if (vm.count("help-module"))
  {
    const std::string &mod(vm["help-module"].as<std::string>());
    if (mod == "network") std::cout << network_opts;
    else if (mod == "client" ) std::cout << client_opts;
    else if (mod == "logging") std::cout << logging_opts;
    else
    {
      std::cout << "Available modules are:" << std::endl
                << "\t" << "network" << std::endl
                << "\t" << "logging" << std::endl
                << "\t" << "client" << std::endl
                << "use --help-module=module to get more information" << std::endl;
    }
    return 0;
  }

  fhg::log::getLogger().addAppender(
    fhg::log::Appender::ptr_t(
      new fhg::log::FileAppender("logfile"
                               , "sdpac.log"
                               , std::ios_base::app
                               | std::ios_base::binary
                               | std::ios_base::out)))->setFormat(fhg::log::Formatter::Custom("%t %s: %l %p:%L - %m%n"));
  if (vm.count("logging.tostderr"))
  {
    fhg::log::getLogger().addAppender(
      fhg::log::Appender::ptr_t(
        new fhg::log::StreamAppender("console", std::cerr)))->setFormat(fhg::log::Formatter::Custom("%s: %p:%L - %m%n"));
  }
  if (vm.count("quiet"))
  {
    fhg::log::getLogger().setLevel(fhg::log::LogLevel::ERROR);
  }
  else
  {
    fhg::log::getLogger().setLevel(fhg::log::LogLevel::INFO);
  }
  if (vm.count("verbose"))
  {
    if (vm["verbose"].as<int>() > 1) fhg::log::getLogger().setLevel(fhg::log::LogLevel::TRACE);
    if (vm["verbose"].as<int>() > 0) fhg::log::getLogger().setLevel(fhg::log::LogLevel::DEBUG);
  }

  try
  {
    sdpa::client::ClientApi::ptr_t api(sdpa::client::ClientApi::create(vm));
    if (vm.count("version"))
    {
      std::cout << "           "
                << "SDPA - Seismic Data Processing Architecture" << std::endl;
      std::cout << "           "
                << "===========================================" << std::endl;
      std::cout << "                            "
                << "v" << api->version()
                << std::endl
                << "                 "
                << " " << api->copyright()
                << std::endl;
      std::cout << "       "
                << api->contact()
                << std::endl;
      return 0;
    }
    if (vm.count("dumpversion"))
    {
      std::cout << api->version() << std::endl;
      return 0;
    }

    if (vm.count("command") == 0)
    {
      std::cerr << "E: a command is required!" << std::endl;
      std::cerr << "E: type --help to get a list of available options!" << std::endl;
      return 1;
    }
    const std::string &command(vm["command"].as<std::string>());

    std::vector<std::string> args;
    if (vm.count("arg"))
    {
      args = vm["arg"].as<std::vector<std::string> >();
    }

    LOG(INFO, "***************************************************");
    LOG(INFO, "SDPA - Seismic Data Processing Architecture (" << api->version() << ")");
    LOG(INFO, "***************************************************");

    api->configure_network(vm);

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
      std::cout << api->submitJob(sstr.str()) << std::endl;
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
      std::string results(api->retrieveResults(args.front()));
      std::ofstream ofs(vm["output"].as<std::string>().c_str());
      ofs << results;
      std::cout << "stored results in: " << vm["output"].as<std::string>() << std::endl;
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
