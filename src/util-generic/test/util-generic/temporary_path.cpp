// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (throws_if_already_exists)
    {
      auto const path (::boost::filesystem::current_path());

      BOOST_REQUIRE (::boost::filesystem::exists (path));

      testing::require_exception
        ( [&] { temporary_path {path}; }
        , error::path_already_exists (path)
        );
    }

    BOOST_AUTO_TEST_CASE (throws_if_unable_to_create)
    {
      BOOST_REQUIRE_THROW
        ( temporary_path {"/proc/nonexistent"};
        , ::boost::system::system_error
        );
    }

    BOOST_AUTO_TEST_CASE (actually_creates_and_deletes)
    {
      auto const path (::boost::filesystem::unique_path());

      BOOST_REQUIRE (!::boost::filesystem::exists (path));

      {
        temporary_path const temp {path};

        BOOST_REQUIRE (::boost::filesystem::is_directory (path));
      }

      BOOST_REQUIRE (!::boost::filesystem::exists (path));
    }

    BOOST_AUTO_TEST_CASE (generates_filename_if_not_given_one)
    {
      ::boost::filesystem::path path;
      {
        temporary_path const temp;
        path = temp;

        BOOST_REQUIRE (::boost::filesystem::is_directory (path));
      }

      BOOST_REQUIRE (!::boost::filesystem::exists (path));
    }
  }
}
