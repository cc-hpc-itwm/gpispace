// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-rpc/locked_with_info_file.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/read_file.hpp>

#include <boost/format.hpp>

#include <exception>

namespace fhg
{
  namespace rpc
  {
    namespace locked_with_info_file
    {
      namespace error
      {
        failed_to_create_server::failed_to_create_server (decltype (directory) dir)
          : std::runtime_error
              ((::boost::format ("Failed to create server for %1%.") % dir).str())
          , directory (std::move (dir))
        {}
        failed_to_create_client::failed_to_create_client (decltype (directory) dir)
          : std::runtime_error
              ((::boost::format ("Failed to create client for %1%.") % dir).str())
          , directory (std::move (dir))
        {}
        bad_server_description::bad_server_description()
          : std::logic_error ("Failed to parse server description.")
        {}
      }

      server::server ( ::boost::asio::io_service& io_service
                     , service_dispatcher& service_dispatcher
                     , util::filesystem_lock_directory const& lock
                     )
      try
        : service_tcp_provider (io_service, service_dispatcher)
        , _server
          ( static_cast<::boost::filesystem::path> (lock) / "server"
          , util::connectable_to_address_string (local_endpoint().address())
          + ' '
          + std::to_string (local_endpoint().port())
          )
      {}
      catch (...)
      {
        std::throw_with_nested (error::failed_to_create_server (lock));
      }

      client::client
        ( ::boost::asio::io_service& io_service
        , ::boost::filesystem::path const& path
        , util::serialization::exception::serialization_functions functions
        )
      try
        : remote_tcp_endpoint
            ( io_service
            , [&path]() -> std::pair<std::string, unsigned short>
              {
                std::string address;
                unsigned short port;
                {
                  std::istringstream iss
                    (fhg::util::read_file (path / "server"));
                  if (! (iss >> address >> port))
                  {
                    throw error::bad_server_description();
                  }
                }
                return {address, port};
              }()
            , functions
            )
          {}
      catch (...)
      {
        std::throw_with_nested (error::failed_to_create_client (path));
      }
    }
  }
}
