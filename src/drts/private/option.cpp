// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <drts/private/option.hpp>

#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <util-generic/boost/program_options/validators/existing_directory.hpp>
#include <util-generic/boost/program_options/validators/existing_path.hpp>
#include <util-generic/boost/program_options/validators/is_directory_if_exists.hpp>
#include <util-generic/boost/program_options/validators/nonempty_file.hpp>
#include <util-generic/boost/program_options/validators/nonempty_string.hpp>
#include <util-generic/boost/program_options/validators/nonexisting_path.hpp>
#include <util-generic/boost/program_options/validators/nonexisting_path_in_existing_directory.hpp>
#include <util-generic/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/join.hpp>

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

      constexpr char const* const log_directory {"log-directory"};
      constexpr char const* const gspc_home {"gspc-home"};
      constexpr char const* const nodefile {"nodefile"};
      constexpr char const* const application_search_path
        {"application-search-path"};
      constexpr char const* const agent_port {"agent-port"};

#define OPTION_NAME_WORKER_ENV_COPY_VARIABLE "worker-env-copy-variable"
#define OPTION_NAME_WORKER_ENV_COPY_CURRENT "worker-env-copy-current"
#define OPTION_NAME_WORKER_ENV_COPY_FILE "worker-env-copy-file"
      constexpr char const* const worker_env_copy_variable
        {OPTION_NAME_WORKER_ENV_COPY_VARIABLE};
      constexpr char const* const worker_env_copy_current
        {OPTION_NAME_WORKER_ENV_COPY_CURRENT};
      constexpr char const* const worker_env_copy_file
        {OPTION_NAME_WORKER_ENV_COPY_FILE};
      constexpr char const* const worker_env_set_variable
        {"worker-env-set-variable"};

      #if GSPC_WITH_IML
      constexpr char const* const virtual_memory_socket
        {"virtual-memory-socket"};
      constexpr char const* const virtual_memory_port
        {"virtual-memory-port"};
      constexpr char const* const virtual_memory_startup_timeout
        {"virtual-memory-startup-timeout"};
      constexpr char const* const virtual_memory_netdev_id
        {"virtual-memory-netdev-id"};

      constexpr char const* const remote_iml_vmem_socket
        {"remote-iml-vmem-socket"};
      #endif

      constexpr char const* const rif_entry_points_file {"rif-entry-points-file"};
      constexpr char const* const rif_port {"rif-port"};
      constexpr char const* const rif_strategy {"rif-strategy"};
      constexpr char const* const rif_strategy_parameters
        {"rif-strategy-parameters"};
    }

    namespace validators
    {
      using namespace fhg::util::boost::program_options;

      struct env_key
      {
        env_key (std::string value);
        operator std::string() const;
        friend std::ostream& operator<< (std::ostream&, env_key const&);
      private:
        std::string _value;
      };
      void validate
        (::boost::any&, std::vector<std::string> const&, env_key*, int);

      struct env_kvpair
      {
        env_kvpair (std::string value);
        operator std::string() const;
        friend std::ostream& operator<< (std::ostream&, env_kvpair const&);
      private:
        std::string _value;
      };
      void validate
        (::boost::any&, std::vector<std::string> const&, env_kvpair*, int);

      env_key::env_key (std::string value)
        : _value (std::move (value))
      {
        if (_value.find ('=') != std::string::npos)
        {
          throw ::boost::program_options::invalid_option_value
            ("environment key shall not contain '='");
        }
        if (_value.empty())
        {
          throw ::boost::program_options::invalid_option_value
            ("environment key shall not be empty");
        }
      }
      env_key::operator std::string() const
      {
        return _value;
      }
      std::ostream& operator<< (std::ostream& os, env_key const& v)
      {
        return os << v._value;
      }
      void validate
        (::boost::any& r, std::vector<std::string> const& vs, env_key*, int)
      {
        fhg::util::boost::program_options::validate<env_key> (r, vs);
      }

      env_kvpair::env_kvpair (std::string value)
        : _value (std::move (value))
      {
        auto pos (_value.find ('='));
        if (pos == std::string::npos)
        {
          throw ::boost::program_options::invalid_option_value
            ( "environment key-value pair shall contain '=' separating key "
              "and value"
            );
        }
        if (pos == 0)
        {
          throw ::boost::program_options::invalid_option_value
            ("environment key-value pair's key shall not be empty");
        }
      }
      env_kvpair::operator std::string() const
      {
        return _value;
      }
      std::ostream& operator<< (std::ostream& os, env_kvpair const& v)
      {
        return os << v._value;
      }
      void validate
        (::boost::any& r, std::vector<std::string> const& vs, env_kvpair*, int)
      {
        fhg::util::boost::program_options::validate<env_kvpair> (r, vs);
      }
    }
  }


  namespace options
  {
    ::boost::program_options::options_description logging()
    {
      ::boost::program_options::options_description logging ("Logging");

      logging.add_options()
        ( name::log_host
        , ::boost::program_options::value<validators::nonempty_string>()
        , "name of a host running a gspc-monitor with registration enabled"
        )
        ( name::log_port
        , ::boost::program_options::value
          <validators::positive_integral<unsigned short>>()
        , "port on log-host running a gspc-monitor with registration enabled"
        )
        ( name::log_level
        , ::boost::program_options::value<std::string>()
        , "DO NOT USE - log level to use"
        )
        ( name::log_directory
        , ::boost::program_options::value<validators::is_directory_if_exists>()
        , "DO NOT USE - directory where to store drts runtime log information"
        )
        ;

      return logging;
    }

    ::boost::program_options::options_description installation()
    {
      ::boost::program_options::options_description
        installation ("GSPC Installation");

      installation.add_options()
        ( name::gspc_home
        , ::boost::program_options::value<validators::existing_directory>()
        ->required()
        , "gspc installation directory"
        )
        ;

      return installation;
    }

    ::boost::program_options::options_description drts()
    {
      ::boost::program_options::options_description drts ("Runtime system");

      drts.add_options()
        //! \todo let it be a list of existing_directories
        ( name::application_search_path
        , ::boost::program_options::value<validators::existing_directory>()
        , "adds a path to the list of application search paths"
        )
        ( name::agent_port
        , ::boost::program_options::value
          <validators::positive_integral<unsigned short>>()
        , "agent port"
        )
        ( name::worker_env_copy_variable
        , ::boost::program_options::value<std::vector<validators::env_key>>()
            ->default_value ({}, "none")
        , "Copy the given environment variables with the currently exported "
          "value to the remote workers."
        )
        ( name::worker_env_copy_current
        , ::boost::program_options::value<bool>()
            ->implicit_value (true)
            ->default_value (false)
        , "Copy the entire environment from the current process to the remote "
          "worker, with no filter at all. Equivalent to specifying "
          OPTION_NAME_WORKER_ENV_COPY_VARIABLE " for all keys in the current "
          "environment."
        )
        ( name::worker_env_copy_file
        , ::boost::program_options::value<std::vector<validators::existing_path>>()
            ->default_value ({}, "none")
        , "Copy the environment described in the given files to the remote "
          "worker. If multiple files are given which contain the same key, "
          "the used value is unspecified, but one of the given values.\n"
          "A file shall have `key=value` lines. This implies multi-line "
          "values are not possible. The key is defined as everything up "
          "to the first `=`.\n"
          "Overwrites values set via " OPTION_NAME_WORKER_ENV_COPY_VARIABLE
          " and " OPTION_NAME_WORKER_ENV_COPY_CURRENT "."
        )
        ( name::worker_env_set_variable
        , ::boost::program_options::value<std::vector<validators::env_kvpair>>()
            ->default_value ({}, "none")
        , "Set a given environment variable in the remote worker process.\n"
          "Arguments shall be of format `key=value`.\n"
          "Overwrites values set via " OPTION_NAME_WORKER_ENV_COPY_VARIABLE
          ", " OPTION_NAME_WORKER_ENV_COPY_CURRENT " and "
          OPTION_NAME_WORKER_ENV_COPY_FILE "."
        )
        ;

      return drts;
    }

    ::boost::program_options::options_description external_rifd()
    {
      ::boost::program_options::options_description drts
        ("Remote Interface Daemon (externally started)");

      drts.add_options()
        ( name::rif_entry_points_file
        , ::boost::program_options::value<validators::nonempty_file>()
          ->required()
        , "entry point description of running remote-interface daemons"
        )
        ;

      return drts;
    }

    ::boost::program_options::options_description scoped_rifd (int options)
    {
      ::boost::program_options::options_description drts
        ("Remote Interface Daemon (internally started)");

      if (options & rifd::nodefile)
      {
        drts.add_options()
        ( name::nodefile
        , ::boost::program_options::value<validators::existing_path>()->required()
        , "nodefile"
        );
      }
      if (options & rifd::rif_strategy)
      {
        drts.add_options()
        ( name::rif_strategy
        , ::boost::program_options::value<std::string>()->required()
        , ( "strategy used to bootstrap rifd (one of "
          + fhg::util::join (fhg::rif::strategy::available_strategies(), ", ").string()
          + ")"
          ).c_str()
        );
        drts.add_options()
        ( name::rif_strategy_parameters
        , ::boost::program_options::value<std::vector<std::string>>()
          ->default_value (std::vector<std::string>(), "")->required()
        , "parameters passed to bootstrapping strategy"
        );
      }
      if (options & rifd::rif_port)
      {
        drts.add_options()
        ( name::rif_port
        , ::boost::program_options::value
          <fhg::util::boost::program_options::positive_integral<unsigned short>>()
        , "port for rifd to listen on"
        );
      }

      return drts;
    }

    #if GSPC_WITH_IML
    ::boost::program_options::options_description virtual_memory()
    {
      ::boost::program_options::options_description vmem ("Virtual memory");

      vmem.add_options()
        ( name::virtual_memory_socket
        , ::boost::program_options::value
            <validators::nonexisting_path_in_existing_directory>()->required()
        , "socket file to communicate with the virtual memory manager"
        )
        ( name::virtual_memory_port
        , ::boost::program_options::value<validators::positive_integral<unsigned short>>()
        ->required()
        , "internal communication port that shall be used by"
          " the virtual memory manager"
        )
        ( name::virtual_memory_startup_timeout
        , ::boost::program_options::value<validators::positive_integral<unsigned long>>()
        ->required()
        , "timeout in seconds for the virtual memory manager to connect and start up."
        )
        ( name::virtual_memory_netdev_id
        , ::boost::program_options::value<iml::gaspi::NetdevID>()
          ->default_value({})
        , "propose a network device ID to use ('auto' for automatic detection"
          ", or '0' or '1' to select a specific device)"
        )
        ;

      return vmem;
    }
    #endif
  }

  namespace
  {
    template<typename T>
      void set_as ( ::boost::program_options::variables_map& vm
                  , std::string const& option_name
                  , T value
                  , std::string const& value_string
                  )
    {
      std::pair<::boost::program_options::variables_map::iterator, bool> const
        pos_and_success
        ( vm.insert
          ( std::make_pair
            ( option_name
            , ::boost::program_options::variable_value (value, false)
            )
          )
        );

      if (!pos_and_success.second)
      {
        throw std::runtime_error
          (( ::boost::format
             ("Failed to set option '%1%' to '%2%': Found old value '%3%'")
           % option_name
           % value_string
           % pos_and_success.first->second.as<T>()
           ).str()
          );
      }
    }

    template<typename T, typename U = T>
      void set_as_vec ( ::boost::program_options::variables_map& vm
                      , std::string const& option_name
                      , std::vector<U> const& value
                      )
    {
      std::vector<T> value_u {value.begin(), value.end()};
      auto const pos_and_success
        ( vm.emplace ( option_name
                     , ::boost::program_options::variable_value
                         (std::move (value_u), false)
                     )
        );

      if (!pos_and_success.second)
      {
        throw std::runtime_error
          (( ::boost::format
             ("Failed to set option '%1%' to '%2%': Found old value '%3%'")
           % option_name
           % fhg::util::join (value, ",")
           % fhg::util::join
               (pos_and_success.first->second.as<std::vector<T>>(), ",")
           ).str()
          );
      }
    }

    template<typename T>
      void set_as ( ::boost::program_options::variables_map& vm
                  , std::string const& option_name
                  , std::string const& value
                  )
    {
      set_as<T> (vm, option_name, value, value);
    }
  }

#define SET(_name, _type)                                        \
  void set_ ## _name ( ::boost::program_options::variables_map& vm \
                     , _type const& value                        \
                     )
#define SET_PATH(_name, _as)                                            \
  SET (_name, ::boost::filesystem::path)                                  \
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
#define SET_VECTOR(_name, _type, _validator)                  \
  SET (_name, std::vector<_type>)                             \
  {                                                           \
    set_as_vec<_validator, _type> (vm, name::_name, value);   \
  }

#define GET_MAYBE(_name, _type, _as)                                    \
  ::boost::optional<_type> get_ ## _name                                  \
    (::boost::program_options::variables_map const& vm)                   \
  {                                                                     \
    if (vm.count (name::_name))                                         \
    {                                                                   \
      return static_cast<_type> (vm.at (name::_name).as<_as>());        \
    }                                                                   \
                                                                        \
    return ::boost::none;                                                 \
  }

#define GET_PATH(_name, _as) GET_MAYBE (_name, ::boost::filesystem::path, _as)
#define GET_STRING(_name, _as) GET_MAYBE (_name, std::string, _as)
#define GET_POSITIVE_INTEGRAL(_name, _type)                             \
  GET_MAYBE (_name, _type, validators::positive_integral<_type>)
#define GET_VECTOR(_name, _type, _as)                                   \
  ::boost::optional<std::vector<_type>> get_ ## _name                     \
    (::boost::program_options::variables_map const& vm)                   \
  {                                                                     \
    if (vm.count (name::_name))                                         \
    {                                                                   \
      auto const v (vm.at (name::_name).as<std::vector<_as>>());        \
      return std::vector<_type> {v.begin(), v.end()};                   \
    }                                                                   \
                                                                        \
    return ::boost::none;                                                 \
  }

#define REQUIRE(_name, _type, _as)                                      \
  _type require_ ## _name                                               \
    (::boost::program_options::variables_map const& vm)                   \
  {                                                                     \
    if (vm.count (name::_name))                                         \
    {                                                                   \
      return vm.at (name::_name).as<_as>();                             \
    }                                                                   \
                                                                        \
    throw std::logic_error                                              \
      (( ::boost::format ("missing key '%1%' in variables map")           \
       % name::_name                                                    \
       ).str()                                                          \
      );                                                                \
  }

#define REQUIRE_PATH(_name, _as)                \
  REQUIRE (_name, ::boost::filesystem::path, _as)
#define REQUIRE_STRING(_name, _as)              \
  REQUIRE (_name, std::string, _as)
#define REQUIRE_POSITIVE_INTEGRAL(_name, _type)                 \
  REQUIRE (_name, _type, validators::positive_integral<_type>)
#define REQUIRE_VECTOR(_name, _type, _as)                               \
  std::vector<_type> require_ ## _name                                  \
    (::boost::program_options::variables_map const& vm)                   \
  {                                                                     \
    if (vm.count (name::_name))                                         \
    {                                                                   \
      auto const v (vm.at (name::_name).as<std::vector<_as>>());        \
      return std::vector<_type> {v.begin(), v.end()};                   \
    }                                                                   \
                                                                        \
    throw std::logic_error                                              \
      (( ::boost::format ("missing key '%1%' in variables map")           \
       % name::_name                                                    \
       ).str()                                                          \
      );                                                                \
  }

#define ACCESS_PATH(_name, _as)                 \
  SET_PATH (_name, _as)                         \
  GET_PATH (_name, _as)                         \
  REQUIRE_PATH (_name, _as)
#define ACCESS_STRING(_name, _as)               \
  SET_STRING (_name, _as)                       \
  GET_STRING (_name, _as)                       \
  REQUIRE_STRING (_name, _as)
#define ACCESS_POSITIVE_INTEGRAL(_name, _type)  \
  SET_POSITIVE_INTEGRAL (_name, _type)          \
  GET_POSITIVE_INTEGRAL (_name, _type)          \
  REQUIRE_POSITIVE_INTEGRAL (_name, _type)
#define ACCESS_VECTOR(_name, _type, _validator)               \
  SET_VECTOR (_name, _type, _validator)                       \
  GET_VECTOR (_name, _type, _validator)                       \
  REQUIRE_VECTOR (_name, _type, _validator)
#define ACCESS_BOOL(_name)                                    \
  SET (_name, bool)                                           \
  {                                                           \
    set_as (vm, name::_name, value, std::to_string (value));  \
  }                                                           \
  GET_MAYBE (_name, bool, bool)                               \
  REQUIRE (_name, bool, bool)

  ACCESS_VECTOR (worker_env_copy_variable, std::string, validators::env_key)
  ACCESS_BOOL (worker_env_copy_current)
  ACCESS_VECTOR
    (worker_env_copy_file, ::boost::filesystem::path, validators::existing_path)
  ACCESS_VECTOR (worker_env_set_variable, std::string, validators::env_kvpair)

  ACCESS_STRING (log_host, validators::nonempty_string)
  ACCESS_POSITIVE_INTEGRAL (log_port, unsigned short)
  ACCESS_STRING (log_level, std::string)

  ACCESS_PATH (log_directory, validators::is_directory_if_exists)
  ACCESS_PATH (gspc_home, validators::existing_directory)
  ACCESS_PATH (nodefile, validators::existing_path)
  ACCESS_PATH (application_search_path, validators::existing_directory)
  ACCESS_POSITIVE_INTEGRAL (agent_port, unsigned short)

  #if GSPC_WITH_IML
  ACCESS_PATH ( remote_iml_vmem_socket
              , validators::existing_path
              )

  ACCESS_PATH ( virtual_memory_socket
              , validators::nonexisting_path_in_existing_directory
              )
  ACCESS_POSITIVE_INTEGRAL (virtual_memory_port, unsigned short)
  ACCESS_POSITIVE_INTEGRAL (virtual_memory_startup_timeout, unsigned long)
  SET (virtual_memory_netdev_id, iml::gaspi::NetdevID)
  {
    set_as<iml::gaspi::NetdevID>
      (vm, name::virtual_memory_netdev_id, value.to_string());
  }
  GET_MAYBE
    (virtual_memory_netdev_id, iml::gaspi::NetdevID, iml::gaspi::NetdevID)
  REQUIRE
    (virtual_memory_netdev_id, iml::gaspi::NetdevID, iml::gaspi::NetdevID)
  #endif

  ACCESS_PATH (rif_entry_points_file, validators::nonempty_file)
  ACCESS_POSITIVE_INTEGRAL (rif_port, unsigned short)
  ACCESS_STRING (rif_strategy, std::string)

  GET_MAYBE (rif_strategy_parameters, std::vector<std::string>, std::vector<std::string>)
  REQUIRE (rif_strategy_parameters, std::vector<std::string>, std::vector<std::string>)
  char const* name_rif_strategy_parameters()
  {
    return name::rif_strategy_parameters;
  }

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
