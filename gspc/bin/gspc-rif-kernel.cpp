// bernd.loerwald@itwm.fraunhofer.de

#include <gspc/rif/daemon.hpp>

#include <fhg/plugin/core/license.hpp>
#include <fhg/util/daemonize.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/program_options.hpp>

#include <string>
#include <vector>

int main(int ac, char **av)
{
  FHGLOG_SETUP();
  fhg::log::Logger::ptr_t logger (fhg::log::Logger::get());

  namespace po = boost::program_options;

  po::options_description desc("options");

  const size_t nthreads (4);
  std::string netd_url;

  desc.add_options()
    ("help,h", "this message")
    ("netd_url", po::value<std::string>(&netd_url), "url to listen on")
    ;

  po::variables_map vm;
  try
  {
    po::store( po::command_line_parser(ac, av)
             . options(desc).run()
             , vm
             );
  }
  catch (std::exception const &ex)
  {
    LLOG (ERROR, logger, "invalid command line: " << ex.what());
    LLOG (ERROR, logger, "use " << av[0] << " --help to get a list of options");
    return EXIT_FAILURE;
  }
  po::notify (vm);

  if (vm.count("help"))
  {
    LLOG (INFO, logger, av[0] << " [options]");
    LLOG (INFO, logger, desc);
    return EXIT_SUCCESS;
  }

  fhg::plugin::magically_check_license (logger);

  fhg::util::fork_and_daemonize_child_and_abandon_parent();

  fhg::util::thread::event<> stop_requested;
  const boost::function<void()> request_stop
    (boost::bind (&fhg::util::thread::event<>::notify, &stop_requested));


  const gspc::rif::daemon rif (request_stop, logger, nthreads, netd_url);


  fhg::util::signal_handler_manager signal_handlers;

  signal_handlers.add_log_backtrace_and_exit_for_critical_errors (logger);

  //! \note boost::bind allows ignoring the three parameters passed
  signal_handlers.add (SIGTERM, boost::bind (request_stop));
  signal_handlers.add (SIGINT, boost::bind (request_stop));


  stop_requested.wait();

  return 0;
}
