// alexander.petry@itwm.fraunhofer.de

#include <fhg/plugin/core/kernel.hpp>
#include <fhg/plugin/core/license.hpp>
#include <fhg/util/daemonize.hpp>
#include <fhg/util/pidfile_writer.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/split.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/foreach.hpp>

int setup_and_run_fhgkernel ( bool daemonize
                            , bool keep_going
                            , std::vector<std::string> mods_to_load
                            , std::vector<std::string> config_vars
                            , std::string pidfile
                            , std::string kernel_name
                            , fhg::core::kernel_t::search_path_t search_path
                            , fhg::log::Logger::ptr_t logger
                            )
{
  fhg::plugin::magically_check_license (logger);

  if (not pidfile.empty())
  {
    fhg::util::pidfile_writer const pidfile_writer (pidfile);

    if (daemonize)
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent();
    }

    pidfile_writer.write();
  }
  else
  {
    if (daemonize)
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent();
    }
  }

  fhg::core::kernel_t kernel (kernel_name, search_path);

  BOOST_FOREACH (std::string const & p, config_vars)
  {
    typedef std::pair<std::string,std::string> key_val_t;
    key_val_t kv (fhg::util::split_string(p, "="));
    if (kv.first.empty())
    {
      LLOG (WARN, logger, "invalid config variable: must not be empty");
    }
    else
    {
      DLLOG (TRACE, logger, "setting " << kv.first << " to " << kv.second);
      kernel.put(kv.first, kv.second);
    }
  }

  BOOST_FOREACH (std::string const & p, mods_to_load)
  {
    try
    {
      int ec = kernel.load_plugin (p);
      if (ec != 0)
        throw std::runtime_error (strerror (ec));
    }
    catch (std::exception const &ex)
    {
      LLOG (ERROR, logger, "could not load `" << p << "' : " << ex.what());
      if (! keep_going)
      {
        throw;
      }
    }
  }

  fhg::util::signal_handler_manager signal_handlers;

  signal_handlers.add_log_backtrace_and_exit_for_critical_errors (logger);

  signal_handlers.add (SIGTERM, boost::bind (&fhg::core::kernel_t::stop, &kernel));
  signal_handlers.add (SIGINT, boost::bind (&fhg::core::kernel_t::stop, &kernel));

  return kernel.run_and_unload (false);
}
