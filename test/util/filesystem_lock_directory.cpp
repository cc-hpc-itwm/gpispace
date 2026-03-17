#include <gspc/util/filesystem_lock_directory.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/test/unit_test.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <filesystem>
#include <fmt/core.h>

BOOST_AUTO_TEST_CASE (lock_fails_on_existing_path_and_does_not_remove_it)
{
  gspc::testing::temporary_path const path;

  BOOST_REQUIRE (std::filesystem::exists (path));

  gspc::testing::require_exception
    ( [&path]
      {
        gspc::util::filesystem_lock_directory
          {static_cast<std::filesystem::path> (path)};
      }
    , gspc::testing::make_nested
        ( gspc::util::failed_to_create_lock
            (static_cast<std::filesystem::path> (path))
        , std::logic_error
            { fmt::format
                ( "Temporary path {} already exists."
                , static_cast<std::filesystem::path> (path)
                )
            }
        )
    );

  BOOST_REQUIRE (std::filesystem::exists (path));
}

BOOST_AUTO_TEST_CASE (lock_creates_path_and_removes_it)
{
  std::filesystem::path const path
    {gspc::testing::unique_path()};

  BOOST_REQUIRE (!std::filesystem::exists (path));

  {
    gspc::util::filesystem_lock_directory const lock1 (path);

    BOOST_REQUIRE (std::filesystem::exists (path));
  }

  BOOST_REQUIRE (!std::filesystem::exists (path));
}

BOOST_AUTO_TEST_CASE (second_lock_fails)
{
  std::filesystem::path const path
    {gspc::testing::unique_path()};

  gspc::util::filesystem_lock_directory const lock (path);

  gspc::testing::require_exception
    ( [&path]
      {
        gspc::util::filesystem_lock_directory {path};
      }
    , gspc::testing::make_nested
        ( gspc::util::failed_to_create_lock (path)
        , std::logic_error
            { fmt::format
                ( "Temporary path {} already exists."
                , path
                )
            }
        )
    );
}
