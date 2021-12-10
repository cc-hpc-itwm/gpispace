// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <boost/test/unit_test.hpp>

#include <util-generic/system.hpp>

#include <util-generic/read_file.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace
    {
      struct with_command
      {
      public:
        with_command()
        {
          BOOST_REQUIRE_EQUAL
            (::boost::unit_test::framework::master_test_suite().argc, 2);

          _ = ::boost::unit_test::framework::master_test_suite().argv[1];
        }

        std::string const& command() { return _; }

      private:
        std::string _;
      };
    }

    BOOST_FIXTURE_TEST_CASE
      (system_executes_a_command, with_command)
    {
      auto const path (::boost::filesystem::unique_path());
      temporary_file const _temp (path);

      auto const content ( testing::random<std::string>{}
                             (testing::random<std::string>::identifier{})
                         );

      system (::boost::format ("%1% %2% %3%") % command() % path % content);

      BOOST_REQUIRE_EQUAL (read_file (path), content);
    }

    namespace
    {
      std::runtime_error einval (std::string command)
      {
        return std::runtime_error
          { ( ::boost::format ("Could not execute '%1%': %2%")
            % command
            % std::strerror (EINVAL)
            ).str()
          };
      }
    }

    BOOST_FIXTURE_TEST_CASE
      (failure_throws_with_error_description, with_command)
    {
      testing::require_exception
        ( [this]
          {
            system (command());
          }
        , einval (command())
        );
    }

    BOOST_FIXTURE_TEST_CASE
      (description_is_nested_around_error, with_command)
    {
      auto const description ( testing::random<std::string>{}
                                 (testing::random<std::string>::identifier{})
                             );

      testing::require_exception
        ( [this, description]
          {
            system<std::logic_error> (description, command());
          }
        , testing::make_nested
          ( std::logic_error ("Could not " + description)
          , einval (command())
          )
        );
    }
  }
}
