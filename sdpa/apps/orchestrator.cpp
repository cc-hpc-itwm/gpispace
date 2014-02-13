#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <csignal>

#include <fhglog/fhglog.hpp>

#include <boost/program_options.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <boost/filesystem/path.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhg/util/daemonize.hpp>
#include <fhg/util/pidfile_writer.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <boost/tokenizer.hpp>

namespace bfs = boost::filesystem;
namespace po = boost::program_options;
using namespace std;

static const int EX_STILL_RUNNING = 4;

int main (int argc, char **argv)
{
    string orchName;
    string orchUrl;
    string kvsUrl;
    string pidfile;

    FHGLOG_SETUP();

    po::options_description desc("Allowed options");
    desc.add_options()
       ("help,h", "Display this message")
       ("name,n", po::value<std::string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
       ("url,u",  po::value<std::string>(&orchUrl)->default_value("localhost"), "Orchestrator's url")
       ("kvs_url,k",  po::value<string>()->required(), "The kvs daemon's url")
       ("pidfile", po::value<std::string>(&pidfile)->default_value(pidfile), "write pid to pidfile")
       ("daemonize", "daemonize after all checks were successful")
       ;

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

    fhg::log::Logger::ptr_t logger (fhg::log::Logger::get (orchName));

    if (vm.count("help"))
    {
      LLOG (ERROR, logger, "usage: orchestrator [options] ....");
      LLOG (ERROR, logger, desc);
      return 0;
    }

    po::notify(vm);

  std::vector< std::string > vec;

  {
    boost::char_separator<char> sep(":");
    boost::tokenizer<boost::char_separator<char> > tok(vm["kvs_url"].as<std::string>(), sep);

    vec.assign(tok.begin(),tok.end());

    if( vec.size() != 2 )
    {
      throw std::runtime_error
        ("Invalid kvs url.  Please specify it in the form <hostname (IP)>:<port>!");
    }
  }

  const std::string kvs_host (vec[0]);
  const std::string kvs_port (vec[1]);

  fhg::com::kvs::get_or_create_global_kvs ( kvs_host, kvs_port, boost::posix_time::seconds(120), 1);

    if (not pidfile.empty())
    {
      fhg::util::pidfile_writer const pidfile_writer (pidfile);

      if (vm.count ("daemonize"))
      {
        fhg::util::fork_and_daemonize_child_and_abandon_parent();
      }

      pidfile_writer.write();
    }
    else
    {
      if (vm.count ("daemonize"))
      {
        fhg::util::fork_and_daemonize_child_and_abandon_parent();
      }
    }

  const sdpa::daemon::Orchestrator orchestrator (orchName, orchUrl);


  fhg::util::thread::event<> stop_requested;
  const boost::function<void()> request_stop
    (boost::bind (&fhg::util::thread::event<>::notify, &stop_requested));

  fhg::util::signal_handler_manager signal_handlers;

  signal_handlers.add_log_backtrace_and_exit_for_critical_errors (logger);

  signal_handlers.add (SIGTERM, boost::bind (request_stop));
  signal_handlers.add (SIGINT, boost::bind (request_stop));


  stop_requested.wait();
}
