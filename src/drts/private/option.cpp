// mirko.rahn@itwm.fraunhofer.de

#include <drts/private/option.hpp>
#include <drts/drts.hpp>

#include <fhg/util/boost/program_options/validators/existing_directory.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/is_directory_if_exists.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>

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
        ( name::virtual_memory_startup_timeout
        , boost::program_options::value<validators::positive_integral<unsigned long>>()
        ->required()
        , "timeout in seconds for the virtual memory manager to connect and start up."
        )
        ;

      return vmem;
    }
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
  get_virtual_memory_startup_timeout (boost::program_options::variables_map const& vm)
  {
    return vm[options::name::virtual_memory_startup_timeout].as<validators::positive_integral<unsigned long>>();
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
