// mirko.rahn@itwm.fraunhofer.de

#include <drts/drts.hpp>
#include <drts/system.hpp>

#include <fhg/util/boost/program_options/validators/executable.hpp>
#include <fhg/util/boost/program_options/validators/existing_directory.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/is_directory_if_exists.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>

#include <boost/format.hpp>

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

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

  scoped_runtime_system::scoped_runtime_system
    ( std::string const& boot_options
    , boost::program_options::variables_map const& vm
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

    command_boot << boot_options;

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
    }
  }

  scoped_runtime_system::~scoped_runtime_system()
  {
    system ( ( boost::format ("%1% -s %2% stop")
             % (_gspc_home / "bin" / "sdpa")
             % _state_directory
             ).str()
           , "stop runtime system"
           );
  }
}
