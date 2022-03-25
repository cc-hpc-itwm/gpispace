// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
        ( temporary_path
            {"/we hopefully don't/run as root and this is/nonexistent"};
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
