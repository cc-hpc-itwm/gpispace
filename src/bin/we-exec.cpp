#include <drts/worker/drts.hpp>

#include <fhg/revision.hpp>
#include <fhg/util/join.hpp>
#include <fhg/util/make_unique.hpp>
#include <fhg/util/print_exception.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/split.hpp>

#include <plugin/core/kernel.hpp>

#include <sdpa/client.hpp>
#include <sdpa/daemon/agent/Agent.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <mutex>

int main (int argc, char **argv)
try
{
  boost::asio::io_service remote_log_io_service;
  FHGLOG_SETUP (remote_log_io_service);
  fhg::log::Logger::ptr_t logger (fhg::log::Logger::get());

  boost::program_options::options_description desc ("options");

  std::vector<std::string> config_vars;
  std::string path_to_act;
  std::vector<std::string> mod_path;
  std::vector<std::string> plugin_path;
  std::size_t num_worker (8);
  std::string output;
  bool vmem_enabled (false);

  desc.add_options()
    ("help,h", "this message")
    ("version,V", "print version information")
    ( "net"
    , boost::program_options::value<std::string> (&path_to_act)
      ->default_value ("-")
    , "path to encoded activity or - for stdin"
    )
    ( "mod-path,L"
    , boost::program_options::value<std::vector<std::string>> (&mod_path)
    , "where can modules be located"
    )
    ( "plugin-path"
    , boost::program_options::value<std::vector<std::string>> (&plugin_path)
    , "where can plugins be located"
    )
    ( "worker"
    , boost::program_options::value<std::size_t> (&num_worker)
      ->default_value (num_worker)
    , "number of workers"
    )
    ( "output,o"
    , boost::program_options::value<std::string> (&output)
      ->default_value (output)
    , "where to write the result pnet to"
    )
    ( "set,s"
    , boost::program_options::value<std::vector<std::string>> (&config_vars)
    , "set a parameter to a value key=value"
    )
    ( "vmem-enabled"
    , boost::program_options::value<bool> (&vmem_enabled)
      ->default_value (vmem_enabled)->implicit_value (true)
    , "whether or not vmem is enabled. requires --worker == 1. requires --vmem-shm-size and --vmem-socket"
    )
    ( "vmem-shm-size"
    , boost::program_options::value<unsigned long>()
    , "local memory size"
    )
    ( "vmem-socket"
    , boost::program_options::value<std::string>()
    , "local gpi-space binary's socket"
    )
    ;

  boost::program_options::positional_options_description p;
  p.add ("input", -1);

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser(argc, argv)
    . options(desc).positional(p).run()
    , vm
    );

  if (vm.count ("help"))
  {
    std::cout << desc << std::endl;
    return 0;
  }

  if (vm.count ("version"))
  {
    std::cout << fhg::project_info ("Parallel Workflow Execution");

    return 0;
  }

  boost::program_options::notify (vm);

  std::string const host ("localhost");

  std::string const orchestrator_name ("orchestrator");
  std::string const agent_name ("agent");

  boost::asio::io_service orchestrator_peer_io_service;
  boost::asio::io_service orchestrator_rpc_io_service;
  sdpa::daemon::Orchestrator const orchestrator
    ( orchestrator_name
    , host
    , orchestrator_peer_io_service
    , orchestrator_rpc_io_service
    );

  boost::asio::io_service agent_peer_io_service;
  sdpa::daemon::Agent const agent
    ( agent_name
    , host
    , agent_peer_io_service
    , vm.count ("vmem-socket")
      ? boost::make_optional (boost::filesystem::path (vm["vmem-socket"].as<std::string>()))
      : boost::none
    , {std::make_tuple (orchestrator_name, orchestrator.peer_host(), orchestrator.peer_port())}
    , boost::none
    );

  std::vector<std::unique_ptr<boost::strict_scoped_thread<boost::join_if_joinable>>>
    workers_list;

  std::map<std::string, std::string> config_variables;
  for (std::string const& config_var : config_vars)
  {
    std::pair<std::string, std::string> const key_value
      (fhg::util::split_string (config_var, '='));

    if (key_value.first.empty())
    {
      throw std::runtime_error ("invalid config variable: must not be empty");
    }

    config_variables.insert (key_value);
  }

  config_variables.emplace ( "plugin.drts.master"
                           , agent_name
                           + "%" + std::string (agent.peer_host())
                           + "%" + std::string (agent.peer_port())
                           );
  config_variables.emplace
    ("plugin.drts.library_path", fhg::util::join (mod_path, ":"));

  if (vmem_enabled)
  {
    if (num_worker != 1)
    {
      throw std::runtime_error ("enabled vmem but has more than one worker");
    }

    config_variables.emplace
      ( "plugin.gpi_compat.shm_size"
      , std::to_string (vm["vmem-shm-size"].as<unsigned long>())
      );
    config_variables.emplace
      ("plugin.gpi.socket", vm["vmem-socket"].as<std::string>());
  }

  std::vector<std::function<void()>> request_stops (num_worker);

  fhg::util::signal_handler_manager signal_handlers;
  fhg::util::scoped_log_backtrace_and_exit_for_critical_errors const
    crit_error_handler (signal_handlers, logger);

  std::mutex mutex_signal_manager;

  while (num_worker --> 0)
  {
    workers_list.emplace_back
      ( fhg::util::make_unique
        <boost::strict_scoped_thread<boost::join_if_joinable>>
        ( [ vmem_enabled
          , config_variables
          , &plugin_path
          , num_worker
          , &logger
          , &request_stops
          , &signal_handlers
          , &mutex_signal_manager
          ]() mutable
        {
          fhg::core::wait_until_stopped waiter;
          const std::function<void()> request_stop (waiter.make_request_stop());

          std::unique_lock<std::mutex> mutex_lock (mutex_signal_manager);

          fhg::util::scoped_signal_handler const SIGTERM_handler
            (signal_handlers, SIGTERM, std::bind (request_stop));
          fhg::util::scoped_signal_handler const SIGINT_handler
            (signal_handlers, SIGINT, std::bind (request_stop));

          mutex_lock.unlock();

          request_stops[num_worker] = request_stop;

          config_variables["kernel_name"]
            = "worker_" + std::to_string (num_worker);

          fhg::core::kernel_t kernel
            (plugin_path, request_stop, config_variables);

          if (vmem_enabled)
          {
            kernel.load_plugin_by_name ("gpi");
            kernel.load_plugin_by_name ("gpi_compat");
          }

          boost::asio::io_service peer_io_service;
          if (config_variables.count ("plugin.drts.gui_url"))
          {
            boost::asio::io_service gui_io_service;
            DRTSImpl const plugin
              ( request_stop
              , peer_io_service
              , std::pair<std::string, boost::asio::io_service&>
                (config_variables.at ("plugin.drts.gui_url"), gui_io_service)
              , config_variables
              );
            waiter.wait();
          }
          else
          {
            DRTSImpl const plugin ( request_stop
                                  , peer_io_service
                                  , boost::none
                                  , config_variables
                                  );
            waiter.wait();
          }
        }
        )
      );
  }

  we::type::activity_t const activity
    ( path_to_act == "-"
    ? we::type::activity_t (std::cin)
    : we::type::activity_t (boost::filesystem::path (path_to_act))
    );

  boost::asio::io_service client_peer_io_service;
  sdpa::client::Client client ( orchestrator.peer_host()
                              , orchestrator.peer_port()
                              , client_peer_io_service
                              );

  sdpa::job_id_t const job_id (client.submitJob (activity.to_string()));

  sdpa::client::job_info_t UNUSED_job_info;
  sdpa::status::code const rc
    (client.wait_for_terminal_state (job_id, UNUSED_job_info));

  std::string const result (client.retrieveResults (job_id));

  client.deleteJob (job_id);

  for (std::function<void()> const& request_stop : request_stops)
  {
    request_stop();
  }

  switch (rc)
  {
  case sdpa::status::FINISHED:
    if (output.size())
    {
      if (output == "-")
      {
        std::cout << result;
      }
      else
      {
        boost::filesystem::ofstream ofs (output);
        ofs << result;
      }
    }
    break;

  case sdpa::status::FAILED:
  case sdpa::status::CANCELED:
    break;

  default:
    throw std::logic_error ("STRANGE: is_running is not consistent!?");
  }

  return rc;
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "");
  return 1;
}
