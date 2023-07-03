// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/RuntimeSystem.hpp>

#include <iml/detail/option.hpp>
#include <iml/detail/startup_and_shutdown.hpp>

#include <util-generic/boost/program_options/validators/nonexisting_path_in_existing_directory.hpp>
#include <util-generic/boost/program_options/validators/positive_integral.hpp>

#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

namespace iml
{
  RuntimeSystem::RuntimeSystem ( rif::EntryPoints const& entry_points
                               , ::boost::filesystem::path socket
                               , unsigned short port
                               , std::chrono::seconds timeout
                               , gaspi::NetdevID netdev_id
                               , std::ostream& output
                               )
    : _processes_storage (std::make_unique<ProcessesStorage> (output))
  {
    _processes_storage->startup
      ( socket
      , timeout
      , port
      , netdev_id
      , entry_points
      , output
      );
  }

  RuntimeSystem::~RuntimeSystem() = default;

  namespace
  {
    namespace option
    {
      namespace validators = fhg::util::boost::program_options;

      constexpr auto const name_socket ("iml-vmem-socket");
      using type_socket = ::boost::filesystem::path;
      using validator_socket = validators::nonexisting_path_in_existing_directory;
      constexpr auto const name_port ("iml-vmem-port");
      using type_port = unsigned short;
      using validator_port = validators::positive_integral<type_port>;
      constexpr auto const name_startup_timeout ("iml-vmem-startup-timeout");
      using type_startup_timeout = std::chrono::seconds;
      using validator_startup_timeout = validators::positive_integral<unsigned long>;
      constexpr auto const name_netdev_id ("iml-vmem-netdev-id");
      using type_netdev_id = gaspi::NetdevID;
      using validator_netdev_id = type_netdev_id;
    }
  }

  RuntimeSystem::RuntimeSystem
      ( rif::EntryPoints const& entry_points
      , ::boost::program_options::variables_map const& vm
      , std::ostream& output
      )
    : RuntimeSystem
        ( entry_points
        , detail::require<option::type_socket, option::validator_socket>
            (vm, option::name_socket)
        , detail::require<option::type_port, option::validator_port>
            (vm, option::name_port)
        , detail::require<option::type_startup_timeout, option::validator_startup_timeout>
            (vm, option::name_startup_timeout)
        , detail::require<option::type_netdev_id, option::validator_netdev_id>
            (vm, option::name_netdev_id)
        , output
        )
  {}

  ::boost::program_options::options_description RuntimeSystem::options()
  {
    ::boost::program_options::options_description options ("Virtual memory");

    options.add_options()
      ( option::name_socket
      , ::boost::program_options::value<option::validator_socket>()
        ->required()
      , "socket file to communicate with the virtual memory manager"
      );

    options.add_options()
      ( option::name_port
      , ::boost::program_options::value<option::validator_port>()
        ->required()
      , "internal communication port that shall be used by"
        " the virtual memory manager"
      );

    options.add_options()
      ( option::name_startup_timeout
      , ::boost::program_options::value<option::validator_startup_timeout>()
        ->required()
      , "timeout in seconds for the virtual memory manager to connect and "
        "start up."
      );

    options.add_options()
      ( option::name_netdev_id
      , ::boost::program_options::value<option::type_netdev_id>()
        ->default_value({})
      , "propose a network device ID to use ('auto' for automatic detection"
        ", or '<device-id> (e.g. '0', '1', ...) to select a specific device)"
      );

    return options;
  }

  void RuntimeSystem::set_socket
    (::boost::program_options::variables_map& vm, option::type_socket value)
  {
    detail::set<option::type_socket, option::validator_socket>
      ( vm, option::name_socket, value
      , [] (option::type_socket const& x)
        {
          return x.string();
        }
      );
  }

  void RuntimeSystem::set_port
    (::boost::program_options::variables_map& vm, option::type_port value)
  {
    detail::set<option::type_port, option::validator_port>
      ( vm, option::name_port, value
      , [] (option::type_port const& x)
        {
          return std::to_string (x);
        }
      );
  }

  void RuntimeSystem::set_startup_timeout
    (::boost::program_options::variables_map& vm, option::type_startup_timeout value)
  {
    detail::set<option::type_startup_timeout, option::validator_startup_timeout>
      ( vm, option::name_startup_timeout, value.count()
      , [] (option::type_startup_timeout const& x)
        {
          return std::to_string (x.count());
        }
      );
  }

  void RuntimeSystem::set_netdev_id
    (::boost::program_options::variables_map& vm, option::type_netdev_id value)
  {
    detail::set<option::type_netdev_id, option::validator_netdev_id>
      ( vm, option::name_netdev_id, value
      , [] (option::type_netdev_id const& x)
        {
          return x.to_string();
        }
      );
  }
}
