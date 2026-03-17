#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/temporary_path.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/test/unit_test.hpp>


  namespace gspc::util
  {
    BOOST_AUTO_TEST_CASE (throws_if_already_exists)
    {
      auto const path (std::filesystem::current_path());

      BOOST_REQUIRE (std::filesystem::exists (path));

      gspc::testing::require_exception
        ( [&] { temporary_path {path}; }
        , error::path_already_exists (path)
        );
    }

    BOOST_AUTO_TEST_CASE (throws_if_unable_to_create)
    {
      BOOST_REQUIRE_THROW
        ( temporary_path {"/proc/nonexistent"};
        , std::system_error
        );
    }

    BOOST_AUTO_TEST_CASE (actually_creates_and_deletes)
    {
      std::filesystem::path const path
        ( gspc::testing::unique_path()
        );

      BOOST_REQUIRE (!std::filesystem::exists (path));

      {
        temporary_path const temp {path};

        BOOST_REQUIRE (std::filesystem::is_directory (path));
      }

      BOOST_REQUIRE (!std::filesystem::exists (path));
    }

    BOOST_AUTO_TEST_CASE (generates_filename_if_not_given_one)
    {
      std::filesystem::path path;
      {
        gspc::testing::temporary_path const temp;
        path = temp;

        BOOST_REQUIRE (std::filesystem::is_directory (path));
      }

      BOOST_REQUIRE (!std::filesystem::exists (path));
    }
  }
