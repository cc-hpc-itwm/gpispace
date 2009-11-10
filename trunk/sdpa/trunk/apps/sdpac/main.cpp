#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unistd.h>

#if defined(HAVE_CONFIG_H)
#include <sdpa/sdpa-config.hpp>
#endif

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>

#include <sdpa/client/ClientApi.hpp>
#include <sdpa/util/Config.hpp>

int main (int argc, char **argv) {
  const std::string name(argv[0]);
  namespace su = sdpa::util;

  sdpa::client::config_t cfg = sdpa::client::ClientApi::config();
  cfg.tool_opts().add_options()
    ("output", su::po::value<std::string>()->default_value("sdpac.out"),
     "path to output file")
    ("command", su::po::value<std::string>(),
     "The command that shall be performed. Possible values are:\n\n"
     "submit: \tsubmits a job to an orchestrator, arg must point to the job-description\n"
     "cancel: \tcancels a running job, arg must specify the job-id\n"
     "status: \tqueries the status of a job, arg must specify the job-id\n"
     "results: \tretrieve the results of a job, arg must specify the job-id\n"
     "delete: \tdelete a finished job, arg must specify the job-id\n"
     )
    ;
  cfg.tool_hidden_opts().add_options()
    ("arg", su::po::value<std::vector<std::string> >(),
     "arguments to the command")
    ;
  cfg.positional_opts().add("command", 1).add("arg", -1);

  cfg.parse_command_line(argc, argv);
  cfg.parse_environment();
  cfg.parse_config_file();
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

  fhg::log::getLogger().addAppender(
    fhg::log::Appender::ptr_t(
      new fhg::log::FileAppender("logfile"
                               , cfg.get<std::string>("logging.file")
                               , std::ios_base::app
                               | std::ios_base::binary
                               | std::ios_base::out)))->setFormat(fhg::log::Formatter::Custom("%t %s: %l %p:%L - %m%n"));
  if (cfg.is_set("logging.tostderr"))
  {
    fhg::log::getLogger().addAppender(
      fhg::log::Appender::ptr_t(
        new fhg::log::StreamAppender("console", std::cerr)))->setFormat(fhg::log::Formatter::Custom("%s: %p:%L - %m%n"));
  }
  if (cfg.is_set("quiet"))
  {
    fhg::log::getLogger().setLevel(fhg::log::LogLevel::ERROR);
  }
  else
  {
    fhg::log::getLogger().setLevel(fhg::log::LogLevel::INFO);
  }
  if (cfg.is_set("verbose"))
  {
    int lvl(cfg.get<int>("verbose"));
    if (lvl > 1) fhg::log::getLogger().setLevel(fhg::log::LogLevel::TRACE);
    if (lvl > 0) fhg::log::getLogger().setLevel(fhg::log::LogLevel::DEBUG);
  }

  try
  {
    sdpa::client::ClientApi::ptr_t api(sdpa::client::ClientApi::create(cfg));
    if (cfg.is_set("version"))
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
    if (cfg.is_set("dumpversion"))
    {
      std::cout << api->version() << std::endl;
      return 0;
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

    api->configure_network(cfg);

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
      std::ofstream ofs(cfg.get("output").c_str());
      ofs << results;
      std::cout << "stored results in: " << cfg.get("output") << std::endl;
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
