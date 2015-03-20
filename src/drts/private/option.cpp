// mirko.rahn@itwm.fraunhofer.de

#include <drts/private/option.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <fhg/util/boost/program_options/validators/existing_directory.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/is_directory_if_exists.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_file.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path_in_existing_directory.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/join.hpp>

#include <rif/strategy/meta.hpp>

#include <boost/format.hpp>

#include <exception>

namespace gspc
{
  namespace
  {
    namespace name
    {
      constexpr char const* const log_host {"log-host"};
      constexpr char const* const log_port {"log-port"};
      constexpr char const* const log_level {"log-level"};
      constexpr char const* const gui_host {"gui-host"};
      constexpr char const* const gui_port {"gui-port"};

      constexpr char const* const log_directory {"log-directory"};
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
      constexpr char const* const virtual_memory_startup_timeout
        {"virtual-memory-startup-timeout"};

      constexpr char const* const rif_entry_points_file {"rif-entry-points-file"};
      constexpr char const* const rif_port {"rif-port"};
      constexpr char const* const rif_strategy {"rif-strategy"};
    }
  }

  namespace validators = fhg::util::boost::program_options;

  namespace options
  {
    boost::program_options::options_description logging()
    {
      boost::program_options::options_description logging ("Logging");

      logging.add_options()
        ( name::log_host
        , boost::program_options::value<validators::nonempty_string>()
        , "name of log host"
        )
        ( name::log_port
        , boost::program_options::value
          <validators::positive_integral<unsigned short>>()
        , "port on log-host to log to"
        )
        ( name::log_level
        , boost::program_options::value<std::string>()->default_value ("INFO")
        , "log level to use"
        )
        ( name::log_directory
        , boost::program_options::value<validators::is_directory_if_exists>()
        , "directory where to store drts runtime log information"
        )
        ( name::gui_host
        , boost::program_options::value<validators::nonempty_string>()
        , "name of gui host"
        )
        ( name::gui_port
        , boost::program_options::value
          <validators::positive_integral<unsigned short>>()
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
        //! \todo let it be a list of existing_directories
        ( name::application_search_path
        , boost::program_options::value<validators::existing_directory>()
        , "adds a path to the list of application search paths"
        )
        ;

      return drts;
    }

    boost::program_options::options_description external_rifd()
    {
      boost::program_options::options_description drts
        ("Remote Interface Daemon (externally started)");

      drts.add_options()
        ( name::rif_entry_points_file
        , boost::program_options::value<validators::nonempty_file>()
          ->required()
        , "entry point description of running remote-interface daemons"
        )
        ;

      return drts;
    }

    boost::program_options::options_description scoped_rifd (int options)
    {
      boost::program_options::options_description drts
        ("Remote Interface Daemon (internally started)");

      if (options & rifd::nodefile)
      {
        drts.add_options()
        ( name::nodefile
        , boost::program_options::value<validators::existing_path>()->required()
        , "nodefile"
        );
      }
      if (options & rifd::rif_strategy)
      {
        drts.add_options()
        ( name::rif_strategy
        , boost::program_options::value<std::string>()->required()
        , ( "strategy used to bootstrap rifd (one of "
          + fhg::util::join (fhg::rif::strategy::available_strategies(), ", ")
          + ")"
          ).c_str()
        );
      }
      if (options & rifd::rif_port)
      {
        drts.add_options()
        ( name::rif_port
        , boost::program_options::value
          <fhg::util::boost::program_options::positive_integral<unsigned short>>()
        , "port for rifd to listen on"
        );
      }

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
        , boost::program_options::value
            <validators::nonexisting_path_in_existing_directory>()->required()
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
    set_as<_as> (vm, name::_name, value.string());                      \
  }
#define SET_STRING(_name, _as)                                          \
  SET (_name, std::string)                                              \
  {                                                                     \
    set_as<_as> (vm, name::_name, value);                               \
  }
#define SET_POSITIVE_INTEGRAL(_name, _type)                             \
  SET (_name, _type)                                                    \
  {                                                                     \
    set_as<validators::positive_integral<_type>>                        \
      (vm, name::_name, std::to_string (value));                        \
  }

#define GET_MAYBE(_name, _type, _as)                                    \
  boost::optional<_type> get_ ## _name                                  \
    (boost::program_options::variables_map const& vm)                   \
  {                                                                     \
    if (vm.count (name::_name))                                         \
    {                                                                   \
      return static_cast<_type> (vm.at (name::_name).as<_as>());        \
    }                                                                   \
                                                                        \
    return boost::none;                                                 \
  }

#define GET_PATH(_name, _as) GET_MAYBE (_name, boost::filesystem::path, _as)
#define GET_STRING(_name, _as) GET_MAYBE (_name, std::string, _as)
#define GET_POSITIVE_INTEGRAL(_name, _type)                             \
  GET_MAYBE (_name, _type, validators::positive_integral<_type>)

#define REQUIRE(_name, _type, _as)                                      \
  _type require_ ## _name                                               \
    (boost::program_options::variables_map const& vm)                   \
  {                                                                     \
    if (vm.count (name::_name))                                         \
    {                                                                   \
      return vm.at (name::_name).as<_as>();                             \
    }                                                                   \
                                                                        \
    throw std::logic_error                                              \
      (( boost::format ("missing key '%1%' in variables map")           \
       % name::_name                                                    \
       ).str()                                                          \
      );                                                                \
  }

#define REQUIRE_PATH(_name, _as)                \
  REQUIRE (_name, boost::filesystem::path, _as)
#define REQUIRE_STRING(_name, _as)              \
  REQUIRE (_name, std::string, _as)
#define REQUIRE_POSITIVE_INTEGRAL(_name, _type)                 \
  REQUIRE (_name, _type, validators::positive_integral<_type>)

#define ACCESS_PATH(_name, _as)                 \
  SET_PATH (_name, _as);                        \
  GET_PATH (_name, _as);                        \
  REQUIRE_PATH (_name, _as)
#define ACCESS_STRING(_name, _as)               \
  SET_STRING (_name, _as);                      \
  GET_STRING (_name, _as);                      \
  REQUIRE_STRING (_name, _as)
#define ACCESS_POSITIVE_INTEGRAL(_name, _type)  \
  SET_POSITIVE_INTEGRAL (_name, _type);         \
  GET_POSITIVE_INTEGRAL (_name, _type);         \
  REQUIRE_POSITIVE_INTEGRAL (_name, _type)


  ACCESS_STRING (log_host, validators::nonempty_string);
  ACCESS_POSITIVE_INTEGRAL (log_port, unsigned short);
  ACCESS_STRING (log_level, std::string);
  ACCESS_STRING (gui_host, validators::nonempty_string);
  ACCESS_POSITIVE_INTEGRAL (gui_port, unsigned short);

  ACCESS_PATH (log_directory, validators::is_directory_if_exists);
  ACCESS_PATH (gspc_home, validators::existing_directory);
  ACCESS_PATH (nodefile, validators::existing_path);
  ACCESS_PATH (application_search_path, validators::existing_directory);

  ACCESS_POSITIVE_INTEGRAL (virtual_memory_per_node, unsigned long);
  ACCESS_PATH ( virtual_memory_socket
              , validators::nonexisting_path_in_existing_directory
              );
  ACCESS_POSITIVE_INTEGRAL (virtual_memory_port, unsigned short);
  ACCESS_POSITIVE_INTEGRAL (virtual_memory_startup_timeout, unsigned long);

  ACCESS_PATH (rif_entry_points_file, validators::nonempty_file);
  ACCESS_POSITIVE_INTEGRAL (rif_port, unsigned short);
  ACCESS_STRING (rif_strategy, std::string);

#undef ACCESS_POSITIVE_INTEGRAL
#undef ACCESS_STRING
#undef ACCESS_PATH

#undef REQUIRE_POSITIVE_INTEGRAL
#undef REQUIRE_STRING
#undef REQUIRE_PATH

#undef REQUIRE

#undef GET_POSITIVE_INTEGRAL
#undef GET_STRING
#undef GET_PATH

#undef GET_MAYBE

#undef SET_POSITIVE_INTEGRAL
#undef SET_STRING
#undef SET_PATH
#undef SET
}
