#include <gspc/rpc/locked_with_info_file.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/read_file.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>
#include <exception>



    namespace gspc::rpc::locked_with_info_file
    {
      namespace error
      {
        failed_to_create_server::failed_to_create_server (decltype (directory) dir)
          : std::runtime_error
              {fmt::format ("Failed to create server for {}.", dir)}
          , directory (std::move (dir))
        {}
        failed_to_create_client::failed_to_create_client (decltype (directory) dir)
          : std::runtime_error
              {fmt::format ("Failed to create client for {}.", dir)}
          , directory (std::move (dir))
        {}
        bad_server_description::bad_server_description()
          : std::logic_error ("Failed to parse server description.")
        {}
      }

      server::server ( ::boost::asio::io_service& io_service
                     , service_dispatcher& service_dispatcher
                     , gspc::util::filesystem_lock_directory const& lock
                     )
      try
        : service_tcp_provider (io_service, service_dispatcher)
        , _server
          ( static_cast<std::filesystem::path> (lock) / "server"
          , gspc::util::connectable_to_address_string (local_endpoint().address())
          + ' '
          + std::to_string (local_endpoint().port())
          )
      {}
      catch (...)
      {
        std::throw_with_nested
          (error::failed_to_create_server (static_cast<std::filesystem::path> (lock)));
      }

      client::client
        ( ::boost::asio::io_service& io_service
        , std::filesystem::path const& path
        , gspc::util::serialization::exception::serialization_functions functions
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
                    (gspc::util::read_file (path / "server"));
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
