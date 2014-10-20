// mirko.rahn@itwm.fraunhofer.de

#include <drts/drts.hpp>

#include <drts/private/option.hpp>

#include <fhg/util/boost/program_options/validators/executable.hpp>
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
}
