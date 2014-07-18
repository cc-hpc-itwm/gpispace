// mirko.rahn@itwm.fraunhofer.de

#include <drts/drts.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/type/activity.hpp>
//! \todo eliminate this include (that completes type transition_t::data)
#include <we/type/net.hpp>

#include <gpi-space/pc/client/api.hpp>

#include <sdpa/client.hpp>

#include <fhg/util/boost/program_options/validators/executable.hpp>
#include <fhg/util/boost/program_options/validators/existing_directory.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/is_directory_if_exists.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>

#include <boost/format.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unordered_set>

namespace gspc
{
  namespace validators = fhg::util::boost::program_options;

  namespace options
  {
    boost::program_options::options_description logging()
    {
      boost::program_options::options_description logging ("Logging");

      logging.add_options()
        ( name::log_host
        , boost::program_options::value<validators::nonempty_string>()
        ->required()
        , "name of log host"
        )
        ( name::log_port
        , boost::program_options::value
          <validators::positive_integral<unsigned short>>()->required()
        , "port on log-host to log to"
        )
        ( name::gui_host
        , boost::program_options::value<validators::nonempty_string>()
          ->required()
        , "name of gui host"
        )
        ( name::gui_port
        , boost::program_options::value
          <validators::positive_integral<unsigned short>>()->required()
        , "port on gui-host to send to"
        )
        ;

      return logging;
    }

    boost::program_options::options_description drts()
    {
      boost::program_options::options_description drts ("Runtime system");

      drts.add_options()
        ( name::gspc_home
        , boost::program_options::value<validators::existing_directory>()
        ->required()
        , "gspc installation directory"
        )
        ( name::nodefile
        , boost::program_options::value<validators::existing_path>()->required()
        , "nodefile"
        )
        ( name::state_directory
        , boost::program_options::value<validators::is_directory_if_exists>()
        ->required()
        , "directory where to store drts runtime state information"
        )
        //! \todo let it be a list of existing_directories
        ( name::application_search_path
        , boost::program_options::value<validators::existing_directory>()
        , "adds a path to the list of application search paths"
        )
        ;

      return drts;
    }

    boost::program_options::options_description virtual_memory()
    {
      boost::program_options::options_description vmem ("Runtime system");

      vmem.add_options()
        ( name::virtual_memory_manager
        , boost::program_options::value<validators::executable>()
        , "memory manager, typically installed in the privilegded folder"
        )
        ( name::virtual_memory_per_node
        , boost::program_options::value
          <validators::positive_integral<unsigned long>>()->required()
        , "virtual memory per node in bytes"
        )
        ( name::virtual_memory_socket
        , boost::program_options::value<validators::nonexisting_path>()
        ->required()
        , "virtual memory per node in bytes"
        )
        ;

      return vmem;
    }
  }

  namespace
  {
    std::unordered_set<std::string> read_nodes
      (boost::filesystem::path const& nodefile)
    {
      std::unordered_set<std::string> nodes;

      {
        std::ifstream stream (nodefile.string());

        std::string node;

        while (std::getline (stream, node))
        {
          nodes.insert (node);
        }
      }

      return nodes;
    }

    void system (std::string const& command, std::string const& description)
    {
      if (int ec = std::system (command.c_str()) != 0)
      {
        throw std::runtime_error
          (( boost::format
           ("Could not '%3%': error code '%1%', command was '%2%'")
           % ec
           % command
           % description
           ).str()
          );
      };
    }
  }

  scoped_runtime_system::scoped_runtime_system
    ( boost::program_options::variables_map const& vm
    , std::string const& topology_description
    )
      : _gspc_home
        ( boost::filesystem::canonical
          (vm[options::name::gspc_home].as<validators::existing_directory>())
        )
      , _state_directory
        ( vm[options::name::state_directory]
        . as<validators::is_directory_if_exists>()
        )
      , _nodefile
        ( boost::filesystem::canonical
          (vm[options::name::nodefile].as<validators::existing_path>())
        )
      , _virtual_memory_per_node
        ( vm.count (options::name::virtual_memory_per_node)
        ? boost::make_optional
          ( vm[options::name::virtual_memory_per_node]
          . as<validators::positive_integral<unsigned long>>()
          )
        : boost::none
        )
      , _virtual_memory_socket
        ( vm.count (options::name::virtual_memory_socket)
        ? boost::make_optional
          ( vm[options::name::virtual_memory_socket]
          . as<validators::nonexisting_path>()
          )
        : boost::none
        )
      , _nodes (read_nodes (_nodefile))
      , _virtual_memory_api
        ( _virtual_memory_socket
        ? new gpi::pc::client::api_t (_virtual_memory_socket->string())
        : nullptr
        )
  {
    std::string const log_host
      (vm[options::name::log_host].as<validators::nonempty_string>());
    unsigned short const log_port
      ( vm[options::name::log_port]
      . as<validators::positive_integral<unsigned short>>()
      );
    std::string const gui_host
      (vm[options::name::gui_host].as<validators::nonempty_string>());
    unsigned short const gui_port
      ( vm[options::name::gui_port]
      . as<validators::positive_integral<unsigned short>>()
      );

    std::ostringstream command_boot;

    command_boot
      << (_gspc_home / "bin" / "sdpa")
      << " boot"
      << " -S " << _state_directory
      << " -f " << _nodefile
      << " -l " << log_host << ":" << log_port
      << " -g " << gui_host << ":" << gui_port
      ;

    if (_virtual_memory_per_node)
    {
      boost::filesystem::path const virtual_memory_manager
        ( boost::filesystem::canonical
          ( vm[options::name::virtual_memory_manager]
          . as<validators::executable>()
          )
        );

      command_boot
        << " -x " << virtual_memory_manager
        << " -y " << *_virtual_memory_socket
        << " -m " << *_virtual_memory_per_node
        ;
    }
    else
    {
      command_boot << " -M";
    }

    if (vm.count (options::name::application_search_path))
    {
      for ( boost::filesystem::path const& path
          : { vm[options::name::application_search_path]
            . as<validators::existing_directory>()
            }
          )
      {
        command_boot << " -A " << boost::filesystem::canonical (path);
      }
    }

    command_boot << topology_description;

    system (command_boot.str(), "start runtime system");

    if (_virtual_memory_per_node)
    {
      std::cerr << "waiting for virtual memory manager" << std::flush;

      while (!boost::filesystem::exists (*_virtual_memory_socket))
      {
        std::cerr << "." << std::flush;

        std::this_thread::sleep_for (std::chrono::milliseconds (200));
      }

      std::cerr << " OKAY" << std::endl;

      _virtual_memory_api->start();
    }
  }

  scoped_runtime_system::~scoped_runtime_system()
  {
    delete _virtual_memory_api;

    system ( ( boost::format ("%1% -s %2% stop")
             % (_gspc_home / "bin" / "sdpa")
             % _state_directory
             ).str()
           , "stop runtime system"
           );
  }

  void scoped_runtime_system::put_and_run
    ( boost::filesystem::path const& workflow
    , std::unordered_map<std::string, std::unordered_set<std::string>> const&
        values_on_ports
    ) const
  {
    // taken from bin/pnetput
    we::type::activity_t activity (workflow);

    for ( std::pair<std::string, std::unordered_set<std::string>> const&
            values_on_port
        : values_on_ports
        )
    {
      for (std::string const& value : values_on_port.second)
      {
        activity.add_input
          ( activity.transition().input_port_by_name (values_on_port.first)
          , expr::parse::parser (value).eval_all()
          );
      }
    }

    // taken from pbs/sdpa and bin/sdpac
    std::string const kvs_host ("localhost");
    unsigned short const kvs_port (2439);

    sdpa::client::Client api
      ("orchestrator", kvs_host, std::to_string (kvs_port));

    std::string const job_id (api.submitJob (activity.to_string()));

    std::cerr << "waiting for job " << job_id << std::endl;

    sdpa::client::job_info_t job_info;

    sdpa::status::code const status
      (api.wait_for_terminal_state (job_id, job_info));

    if (sdpa::status::FAILED == status)
    {
      std::cerr << "failed: "
                << "error-message := " << job_info.error_message
                << std::endl;
    }
  }

  vmem_allocation scoped_runtime_system::alloc
    (unsigned long size, std::string const& description) const
  {
    return vmem_allocation (this, size, description);
  }
}
