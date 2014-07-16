#include <drts/worker/drts.hpp>

#include <fhg/revision.hpp>
#include <fhg/util/join.hpp>
#include <fhg/util/make_unique.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/split.hpp>

#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/tcp_server.hpp>

#include <plugin/core/kernel.hpp>

#include <sdpa/client.hpp>
#include <sdpa/daemon/agent/Agent.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <mutex>

int main (int argc, char **argv)
try
{
  FHGLOG_SETUP();
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

  struct kvs_server : boost::noncopyable
  {
    kvs_server (std::string const& host)
      : _io_service_pool (1)
      , _kvs_daemon (boost::none)
      , _tcp_server
        (_io_service_pool.get_io_service(), _kvs_daemon, host, "0", true)
      , _io_thread (&fhg::com::io_service_pool::run, &_io_service_pool)
    {}
    ~kvs_server()
    {
      _tcp_server.stop();
      _io_service_pool.stop();
    }

    std::string port() const
    {
      return boost::lexical_cast<std::string> (_tcp_server.port());
    }

  private:
    fhg::com::io_service_pool _io_service_pool;
    fhg::com::kvs::server::kvsd _kvs_daemon;
    fhg::com::tcp_server _tcp_server;
    boost::scoped_thread<boost::join_if_joinable> _io_thread;
  } const kvs_server (host);

  std::string const kvs_port (kvs_server.port());
  std::string const orchestrator_name ("orchestrator");
  std::string const agent_name ("agent");

  sdpa::daemon::Orchestrator const orchestrator
    (orchestrator_name, host, host, kvs_port);

  sdpa::daemon::Agent const agent
    ( agent_name
    , host
    , host, kvs_port
    , {sdpa::MasterInfo (orchestrator_name)}
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

  config_variables.emplace ("plugin.drts.kvs_host", host);
  config_variables.emplace ("plugin.drts.kvs_port", kvs_port);
  config_variables.emplace ("plugin.drts.master", agent_name);
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

  signal_handlers.add_log_backtrace_and_exit_for_critical_errors (logger);

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

          {
            std::unique_lock<std::mutex> const _ (mutex_signal_manager);

            signal_handlers.add (SIGTERM, std::bind (request_stop));
            signal_handlers.add (SIGINT, std::bind (request_stop));
          }

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

          DRTSImpl const plugin (request_stop, config_variables);

          waiter.wait();
        }
        )
      );
  }

  we::type::activity_t const activity
    ( path_to_act == "-"
    ? we::type::activity_t (std::cin)
    : we::type::activity_t (boost::filesystem::path (path_to_act))
    );

  sdpa::client::Client client (orchestrator_name, host, kvs_port);

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
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  return 1;
}
