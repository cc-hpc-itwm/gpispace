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
#include <fhg/util/make_unique.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <boost/format.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace gspc
{
  namespace validators = fhg::util::boost::program_options;

  namespace options
  {
    namespace name
    {
      constexpr char const* const log_host {"log-host"};
      constexpr char const* const log_port {"log-port"};
      constexpr char const* const log_level {"log-level"};
      constexpr char const* const gui_host {"gui-host"};
      constexpr char const* const gui_port {"gui-port"};

      constexpr char const* const state_directory {"state-directory"};
      constexpr char const* const gspc_home {"gspc-home"};
      constexpr char const* const nodefile {"nodefile"};
      constexpr char const* const application_search_path
      {"application-search-path"};

      constexpr char const* const virtual_memory_per_node
      {"virtual-memory-per-node"};
      constexpr char const* const virtual_memory_socket
      {"virtual-memory-socket"};
      constexpr char const* const virtual_memory_port
      {"virtual-memory-port"};
      constexpr char const* const virtual_memory_timeout
      {"virtual-memory-timeout"};
    }

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
        ( name::log_level
        , boost::program_options::value<std::string>()->default_value ("INFO")
        , "log level to use"
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

    boost::program_options::options_description installation()
    {
      boost::program_options::options_description
        installation ("GSPC Installation");

      installation.add_options()
        ( name::gspc_home
        , boost::program_options::value<validators::existing_directory>()
        ->required()
        , "gspc installation directory"
        )
        ;

      return installation;
    }

    boost::program_options::options_description drts()
    {
      boost::program_options::options_description drts ("Runtime system");

      drts.add_options()
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
      boost::program_options::options_description vmem ("Virtual memory");

      vmem.add_options()
        ( name::virtual_memory_per_node
        , boost::program_options::value
          <validators::positive_integral<unsigned long>>()->required()
        , "virtual memory per node in bytes"
        )
        ( name::virtual_memory_socket
        , boost::program_options::value<validators::nonexisting_path>()
        ->required()
        , "socket file to communicate with the virtual memory manager"
        )
        ( name::virtual_memory_port
        , boost::program_options::value<validators::positive_integral<unsigned short>>()
        ->required()
        , "internal communication port that shall be used by the virtual memory manager"
        )
        ( name::virtual_memory_timeout
        , boost::program_options::value<validators::positive_integral<unsigned long>>()
        ->required()
        , "timeout in milliseconds for the virtual memory manager to connect and start up."
        )
        ;

      return vmem;
    }
  }

  namespace
  {
    std::pair<std::list<std::string>, unsigned long>
      read_nodes (boost::filesystem::path const& nodefile)
    {
      std::unordered_set<std::string> unique_nodes;
      std::list<std::string> nodes;

      {
        std::ifstream stream (nodefile.string());

        std::string node;

        while (std::getline (stream, node))
        {
          unique_nodes.insert (node);
          nodes.emplace_back (node);
        }
      }

      if (unique_nodes.empty())
      {
        throw std::runtime_error
          (( boost::format ("nodefile %1% does not contain nodes")
           % nodefile
           ).str()
          );
      }

      return std::make_pair (nodes, unique_nodes.size());
    }

    void system (std::string const& command, std::string const& description)
    {
      if (int ec = fhg::util::system_with_blocked_SIGCHLD (command.c_str()))
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

  installation::installation
    (boost::program_options::variables_map const& vm)
    : _gspc_home
      ( boost::filesystem::canonical
        (vm[options::name::gspc_home].as<validators::existing_directory>())
      )
  {}

  scoped_runtime_system::scoped_runtime_system
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , std::string const& topology_description
    )
      : _installation (installation)
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
      , _nodes_and_number_of_unique_nodes (read_nodes (_nodefile))
      , _virtual_memory_api
        ( _virtual_memory_socket
        ? fhg::util::make_unique<gpi::pc::client::api_t>
          (_virtual_memory_socket->string())
        : nullptr
        )
  {
    std::ostringstream command_boot;

    command_boot
      << (_installation.gspc_home() / "bin" / "sdpa")
      << " -s " << _state_directory
      << " boot"
      << " -f " << _nodefile;

    if (vm.count (options::name::log_host))
    {
      command_boot << " -l " <<
        (vm[options::name::log_host].as<validators::nonempty_string>());

      if (vm.count (options::name::log_port))
      {
        command_boot << ":" <<
          ( vm[options::name::log_port]
          . as<validators::positive_integral<unsigned short>>()
          );
      }
    }

    if (vm.count (options::name::gui_host))
    {
      command_boot << " -g " <<
        (vm[options::name::gui_host].as<validators::nonempty_string>());

      if (vm.count (options::name::gui_port))
      {
        command_boot << ":" <<
          ( vm[options::name::gui_port]
          . as<validators::positive_integral<unsigned short>>()
          );
      }
    }

    if (_virtual_memory_per_node)
    {
      command_boot
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

    command_boot << " " << topology_description;

    system (command_boot.str(), "start runtime system");

    if (_virtual_memory_per_node)
    {
      _virtual_memory_api->start();
    }
  }

  scoped_runtime_system::~scoped_runtime_system()
  {
    _virtual_memory_api.reset();

    system ( ( boost::format ("%1% -s %2% stop")
             % (_installation.gspc_home() / "bin" / "sdpa")
             % _state_directory
             ).str()
           , "stop runtime system"
           );
  }

  std::multimap<std::string, pnet::type::value::value_type>
    scoped_runtime_system::put_and_run
    ( boost::filesystem::path const& workflow
    , std::multimap< std::string
                   , pnet::type::value::value_type
                   > const& values_on_ports
    ) const
  {
    // taken from bin/pnetput
    we::type::activity_t activity (workflow);

    for ( std::pair< std::string
                   , pnet::type::value::value_type
                   > const& value_on_port
        : values_on_ports
        )
    {
      activity.add_input
        ( activity.transition().input_port_by_name (value_on_port.first)
        , value_on_port.second
        );
    }

    // taken from pbs/sdpa and bin/sdpac
    //! \todo Remove magic: specify filenames instead of relying on
    //! file? Let an c++-ified sdpa-boot() return them.
    std::string const kvs_host
      (fhg::util::read_file (_state_directory / "kvs.host"));
    unsigned short const kvs_port
      ( boost::lexical_cast<unsigned short>
        (fhg::util::read_file (_state_directory / "kvs.port"))
      );

    sdpa::client::Client api
      ("orchestrator", kvs_host, std::to_string (kvs_port));

    std::string const job_id (api.submitJob (activity.to_string()));

    std::cerr << "waiting for job " << job_id << std::endl;

    sdpa::client::job_info_t job_info;

    sdpa::status::code const status
      (api.wait_for_terminal_state (job_id, job_info));

    if (sdpa::status::FAILED == status)
    {
      //! \todo decorate the exception with the most progressed activity
      throw std::runtime_error
        (( boost::format ("Job %1%: failed: error-message := %2%")
         % job_id
         % job_info.error_message
         ).str()
        );
    }

    we::type::activity_t const result_activity (api.retrieveResults (job_id));

    api.deleteJob (job_id);

    std::multimap<std::string, pnet::type::value::value_type> result;

    for ( std::pair<pnet::type::value::value_type, we::port_id_type>
           const& value_on_port
        : result_activity.output()
        )
    {
      result.emplace
        ( result_activity.transition().ports_output()
        . at (value_on_port.second).name()
        , value_on_port.first
        );
    }

    return result;
  }

  vmem_allocation scoped_runtime_system::alloc
    (unsigned long size, std::string const& description) const
  {
    return vmem_allocation (this, size, description);
  }

  namespace
  {
    template<typename T>
      void set_as ( boost::program_options::variables_map& vm
                  , std::string const& option_name
                  , std::string const& value
                  )
    {
      std::pair<boost::program_options::variables_map::iterator, bool> const
        pos_and_success
        ( vm.insert
          ( std::make_pair
            ( option_name
            , boost::program_options::variable_value (T (value), false)
            )
          )
        );

      if (!pos_and_success.second)
      {
        throw std::runtime_error
          (( boost::format
             ("Failed to set option '%1%' to '%2%': Found old value '%3%'")
           % option_name
           % value
           % pos_and_success.first->second.as<T>()
           ).str()
          );
      }
    }
  }

  void set_gspc_home ( boost::program_options::variables_map& vm
                     , boost::filesystem::path const& path
                     )
  {
    set_as<validators::existing_directory>
      (vm, options::name::gspc_home, path.string());
  }
  void set_state_directory ( boost::program_options::variables_map& vm
                           , boost::filesystem::path const& path
                           )
  {
    set_as<validators::is_directory_if_exists>
      (vm, options::name::state_directory, path.string());
  }
  void set_nodefile ( boost::program_options::variables_map& vm
                    , boost::filesystem::path const& path
                    )
  {
    set_as<validators::existing_path>
      (vm, options::name::nodefile, path.string());
  }
  boost::filesystem::path get_nodefile (boost::program_options::variables_map const& vm)
  {
    return vm[options::name::nodefile].as<validators::existing_path>();
  }
  void set_virtual_memory_per_node ( boost::program_options::variables_map& vm
                                   , unsigned long size
                                  )
  {
    set_as<validators::positive_integral<unsigned long>>
      (vm, options::name::virtual_memory_per_node, std::to_string (size));
  }
  unsigned long
  get_virtual_memory_per_node (boost::program_options::variables_map const& vm)
  {
    return vm[options::name::virtual_memory_per_node].as<validators::positive_integral<unsigned long>>();
  }
  unsigned short
  get_virtual_memory_port (boost::program_options::variables_map const& vm)
  {
    return vm[options::name::virtual_memory_port].as<validators::positive_integral<unsigned short>>();
  }
  unsigned long
  get_virtual_memory_timeout (boost::program_options::variables_map const& vm)
  {
    return vm[options::name::virtual_memory_timeout].as<validators::positive_integral<unsigned long>>();
  }

  void set_virtual_memory_socket ( boost::program_options::variables_map& vm
                                 , boost::filesystem::path const& path
                                  )
  {
    set_as<validators::nonexisting_path>
      (vm, options::name::virtual_memory_socket, path.string());
  }
  boost::filesystem::path
  get_not_yet_existing_virtual_memory_socket (boost::program_options::variables_map const& vm)
  {
    return vm[options::name::virtual_memory_socket].as<validators::nonexisting_path>();
  }

  void set_application_search_path ( boost::program_options::variables_map& vm
                                   , boost::filesystem::path const& path
                                  )
  {
    set_as<validators::existing_directory>
      (vm, options::name::application_search_path, path.string());
  }
  void set_log_host ( boost::program_options::variables_map& vm
                    , std::string const& host
                    )
  {
    set_as<validators::nonempty_string> (vm, options::name::log_host, host);
  }
  std::string get_log_host (boost::program_options::variables_map const& vm)
  {
    return vm[options::name::log_host].as<validators::nonempty_string>();
  }
  void set_gui_host ( boost::program_options::variables_map& vm
                    , std::string const& host
                    )
  {
    set_as<validators::nonempty_string> (vm, options::name::gui_host, host);
  }
  std::string get_gui_host (boost::program_options::variables_map const& vm)
  {
    return vm[options::name::gui_host].as<validators::nonempty_string>();
  }
  void set_log_port ( boost::program_options::variables_map& vm
                    , unsigned short port
                    )
  {
    set_as<validators::positive_integral<unsigned short>>
      (vm, options::name::log_port, std::to_string (port));
  }
  unsigned short get_log_port (boost::program_options::variables_map const& vm)
  {
    return vm[options::name::log_port].as<validators::positive_integral<unsigned short>>();
  }
  void set_log_level ( boost::program_options::variables_map& vm
                     , std::string const& level
                     )
  {
    set_as<std::string> (vm, options::name::log_level, level);
  }
  std::string get_log_level (boost::program_options::variables_map const& vm)
  {
    return vm[options::name::log_level].as<std::string>();
  }
  void set_gui_port ( boost::program_options::variables_map& vm
                    , unsigned short port
                    )
  {
    set_as<validators::positive_integral<unsigned short>>
      (vm, options::name::gui_port, std::to_string (port));
  }
  unsigned short get_gui_port (boost::program_options::variables_map const& vm)
  {
    return vm[options::name::gui_port].as<validators::positive_integral<unsigned short>>();
  }
}
