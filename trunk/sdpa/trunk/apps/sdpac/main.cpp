#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>

#include <sdpa/sdpa-config.hpp>
#include <sdpa/client/ClientApi.hpp>

static void usage(const std::string &name)
{
  std::cerr << "usage: " << name << " [-v|-h|-V] command [arguments]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "\tcommands and their arguments are:" << std::endl;
  std::cerr << "\t\tsubmit path-to-job-desc" << std::endl;
  std::cerr << "\t\tcancel job-id" << std::endl;
  std::cerr << "\t\tstatus job-id" << std::endl;
  std::cerr << "\t\tresults job-id" << std::endl;
  std::cerr << "\t\tdelete job-id" << std::endl;
}

int main (int argc, char **argv) {
  const std::string name(argv[0]);

  int idx(1);
  if (idx == argc)
  {
    usage(argv[0]);
    return (1);
  }

  if (argv[idx] == std::string("-h"))
  {
    usage(argv[0]);
    return (0);
  }

  fhg::log::getLogger().addAppender(
    fhg::log::Appender::ptr_t(
      new fhg::log::FileAppender("logfile"
                               , "sdpac.log"
                               , std::ios_base::app
                               | std::ios_base::binary
                               | std::ios_base::out)))->setFormat(fhg::log::Formatter::Custom("%t %s: %l %p:%L - %m%n"));
  fhg::log::getLogger().setLevel(fhg::log::LogLevel::INFO);
  if (argv[idx] == std::string("-v"))
  {
    fhg::log::getLogger().setLevel(fhg::log::LogLevel::TRACE);
    idx++;
  }

  if (idx == argc)
  {
    usage(argv[0]);
    return (0);
  }  

  try
  {
    sdpa::client::ClientApi::ptr_t api(sdpa::client::ClientApi::create("empty config"));
    if (argv[idx] == std::string("-V"))
    {
      std::cerr << "           "
                << "SDPA - Seismic Data Processing Architecture" << std::endl;
      std::cerr << "           "
                << "===========================================" << std::endl;
      std::cerr << "                            "
                << "v" << api->version()
                << std::endl
                << "                 "
                << " " << api->copyright()
                << std::endl;
      std::cerr << "       "
                << api->contact()
                << std::endl;
      return 0;
    }

    LOG(INFO, "***************************************************");
    LOG(INFO, "SDPA - Seismic Data Processing Architecture (" << api->version() << ")");
    LOG(INFO, "***************************************************");

    if (argv[idx] == std::string("submit"))
    {
      if ((idx+1) == argc)
      {
        std::cerr << "submit requires an argument" << std::endl;
        return (1);
      }

      std::ifstream ifs(argv[idx+1]);
      if (! ifs.good())
      {
        std::cerr << "could not open: " << argv[idx+1] << std::endl;
        return (2);
      }

      std::stringstream sstr;
      ifs >> sstr.rdbuf();
      std::cout << api->submitJob(sstr.str()) << std::endl;
    }
    else if (argv[idx] == std::string("cancel"))
    {
      if ((idx+1) == argc)
      {
        std::cerr << "cancel requires an argument" << std::endl;
        return (1);
      }
      api->cancelJob(argv[idx+1]);
    }
    else if (argv[idx] == std::string("status"))
    {
      if ((idx+1) == argc)
      {
        std::cerr << "status requires an argument" << std::endl;
        return (1);
      }
      std::cout << api->queryJob(argv[idx+1]) << std::endl;
    }
    else if (argv[idx] == std::string("results"))
    {
      if ((idx+1) == argc)
      {
        std::cerr << "results requires an argument" << std::endl;
        return (1);
      }
      std::cout << api->retrieveResults(argv[idx+1]) << std::endl;
    }
    else if (argv[idx] == std::string("delete"))
    {
      if ((idx+1) == argc)
      {
        std::cerr << "delete requires an argument" << std::endl;
        return (1);
      }
      api->deleteJob(argv[idx+1]);
    }
    else
    {
      std::cerr << "illegal command: " << argv[idx] << std::endl;
      return (1);
    }
  }
  catch (const sdpa::client::ClientException &ce)
  {
    std::cerr << "failed: " << ce.what() << std::endl;
    return 3;
  }
}
