#include <gspc/rpc/locked_with_info_file.hpp>
#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>

#include <gspc/util/read_file.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <future>
#include <memory>

BOOST_AUTO_TEST_CASE (two_server_on_same_lock_is_impossible)
{
  gspc::rpc::service_dispatcher service_dispatcher;

  std::filesystem::path const path
    {std::filesystem::absolute
      (gspc::testing::unique_path())};

  gspc::util::filesystem_lock_directory const lock (path);

  gspc::util::scoped_boost_asio_io_service_with_threads io_service (1);
  gspc::rpc::locked_with_info_file::server const server
    {io_service, service_dispatcher, lock};

  gspc::testing::require_exception
    ( [&io_service, &service_dispatcher, &lock]
      {
        gspc::rpc::locked_with_info_file::server
          {io_service, service_dispatcher, lock};
      }
    , gspc::testing::make_nested
        ( gspc::rpc::locked_with_info_file::error::failed_to_create_server (path)
        , std::logic_error
          { fmt::format ( "Temporary file {} already exists."
                        , path / "server"
                        )
          }
        )
    );
}

BOOST_AUTO_TEST_CASE (create_client_on_nonexisting_path_fails)
{
  std::filesystem::path const path
    {gspc::testing::unique_path()};

  gspc::testing::require_exception
    ( [&path]
      {
        gspc::util::scoped_boost_asio_io_service_with_threads io_service (1);
        gspc::rpc::locked_with_info_file::client {io_service, path};
      }
    , gspc::testing::make_nested
        ( gspc::rpc::locked_with_info_file::error::failed_to_create_client (path)
        , std::runtime_error
            { fmt::format ("could not open {}", path / "server")
            }
        )
    );
}

BOOST_AUTO_TEST_CASE (create_client_on_bad_description_fails)
{
  gspc::testing::temporary_path const path;
  gspc::util::scoped_file_with_content const file
    {static_cast<std::filesystem::path> (path) / "server", ""};

  gspc::testing::require_exception
    ( [&path]
      {
        gspc::util::scoped_boost_asio_io_service_with_threads io_service (1);
        gspc::rpc::locked_with_info_file::client
          {io_service, static_cast<std::filesystem::path> (path)};
      }
    , gspc::testing::make_nested
        ( gspc::rpc::locked_with_info_file::error::failed_to_create_client
            (static_cast<std::filesystem::path> (path))
        , gspc::rpc::locked_with_info_file::error::bad_server_description()
        )
    );
}

BOOST_AUTO_TEST_CASE (create_client_on_outdated_description_fails)
{
  gspc::util::scoped_boost_asio_io_service_with_threads io_service (1);

  gspc::rpc::service_dispatcher service_dispatcher;

  std::filesystem::path const server_path
    {gspc::testing::unique_path()};

  gspc::util::filesystem_lock_directory const lock (server_path);

  std::unique_ptr<gspc::rpc::locked_with_info_file::server> server
    (std::make_unique<gspc::rpc::locked_with_info_file::server>
      (io_service, service_dispatcher, lock)
    );

  gspc::testing::temporary_path const copy_path;
  gspc::util::scoped_file_with_content const copy_server_file
    { static_cast<std::filesystem::path> (copy_path) / "server"
    , gspc::util::read_file (std::filesystem::path (server_path) / "server")
    };

  server.reset();

  gspc::testing::require_exception
    ( [&io_service, &copy_path]
      {
        gspc::rpc::locked_with_info_file::client client
          {io_service, static_cast<std::filesystem::path> (copy_path)};
      }
    , gspc::rpc::locked_with_info_file::error::failed_to_create_client
        (static_cast<std::filesystem::path> (copy_path))
    );
}

namespace protocol
{
  FHG_RPC_FUNCTION_DESCRIPTION (ping, int (int));
}

BOOST_AUTO_TEST_CASE (client_can_communicate_with_server)
{
  gspc::rpc::service_dispatcher service_dispatcher;
  gspc::rpc::service_handler<protocol::ping> const handler_ping
    ( service_dispatcher
    , [] (int i) { return i + 1; }
    );

  std::filesystem::path const path
    {gspc::testing::unique_path()};

  gspc::util::filesystem_lock_directory const lock (path);

  gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
  gspc::rpc::locked_with_info_file::server const server
    {io_service_server, service_dispatcher, lock};

  gspc::util::scoped_boost_asio_io_service_with_threads io_service_client (1);
  gspc::rpc::locked_with_info_file::client client {io_service_client, path};

  int const s {gspc::testing::random<int>()()};

  BOOST_REQUIRE_EQUAL
    (s + 1, gspc::rpc::sync_remote_function<protocol::ping> {client} (s));
}
