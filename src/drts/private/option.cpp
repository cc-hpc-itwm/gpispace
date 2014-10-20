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
        , "internal communication port that shall be used by"
          " the virtual memory manager"
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

#define SET(_name, _type)                                        \
  void set_ ## _name ( boost::program_options::variables_map& vm \
                     , _type const& value                        \
                     )
#define SET_PATH(_name, _as)                                            \
  SET (_name, boost::filesystem::path)                                  \
  {                                                                     \
    set_as<_as> (vm, options::name::_name, value.string());             \
  }
#define SET_STRING(_name, _as)                                          \
  SET (_name, std::string)                                              \
  {                                                                     \
    set_as<_as> (vm, options::name::_name, value);                      \
  }
#define SET_INTEGRAL(_name, _type)                                      \
  SET (_name, _type)                                                    \
  {                                                                     \
    set_as<validators::positive_integral<_type>>                        \
      (vm, options::name::_name, std::to_string (value));               \
  }

#define GET(_name, _type)                                               \
  boost::optional<_type> get_ ## _name                                  \
    (boost::program_options::variables_map const& vm)

#define GET_MAYBE(_name, _type, _as)                                    \
  GET(_name, _type)                                                     \
  {                                                                     \
    if (vm.count (options::name::_name))                                \
    {                                                                   \
      return static_cast<_type> (vm[options::name::_name].as<_as>());   \
    }                                                                   \
                                                                        \
    return boost::none;                                                 \
  }

#define GET_PATH(_name, _as) GET_MAYBE (_name, boost::filesystem::path, _as)
#define GET_STRING(_name, _as) GET_MAYBE (_name, std::string, _as)
#define GET_INTEGRAL(_name, _type) \
  GET_MAYBE (_name, _type, validators::positive_integral<_type>)

#define ACCESS_PATH(_name, _as)                 \
  SET_PATH (_name, _as); GET_PATH (_name, _as)
#define ACCESS_STRING(_name, _as)                       \
  SET_STRING (_name, _as); GET_STRING (_name, _as)
#define ACCESS_INTEGRAL(_name, _type)                           \
  SET_INTEGRAL (_name, _type); GET_INTEGRAL (_name, _type)


  ACCESS_STRING (log_host, validators::nonempty_string);
  ACCESS_INTEGRAL (log_port, unsigned short);
  ACCESS_STRING (log_level, std::string);
  ACCESS_STRING (gui_host, validators::nonempty_string);
  ACCESS_INTEGRAL (gui_port, unsigned short);

  ACCESS_PATH (state_directory, validators::is_directory_if_exists);
  ACCESS_PATH (gspc_home, validators::existing_directory);
  ACCESS_PATH (nodefile, validators::existing_path);
  ACCESS_PATH (application_search_path, validators::existing_directory);

  ACCESS_INTEGRAL (virtual_memory_per_node, unsigned long);
  ACCESS_PATH (virtual_memory_socket, validators::nonexisting_path);
  ACCESS_INTEGRAL (virtual_memory_port, unsigned short);
  ACCESS_INTEGRAL (virtual_memory_startup_timeout, unsigned long);

#undef ACCESS_INTEGRAL
#undef ACCESS_STRING
#undef ACCESS_PATH

#undef GET_INTEGRAL
#undef GET_STRING
#undef GET_PATH

#undef GET_MAYBE

#undef GET

#undef SET_INTEGRAL
#undef SET_STRING
#undef SET_PATH
#undef SET
}
