// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/system.hpp>

#include <util-generic/read_file.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/filesystem.hpp>

#include <FMT/boost/filesystem/path.hpp>
#include <cerrno>
#include <cstring>
#include <fmt/core.h>
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
            (::boost::unit_test::framework::master_test_suite().argc, 4);

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

      system (fmt::format ("{0} {1} {2}", command(), path, content));

      BOOST_REQUIRE_EQUAL (read_file (path), content);
    }

    namespace
    {
      std::runtime_error einval (std::string command)
      {
        return std::runtime_error
          { fmt::format ( "Could not execute '{0}': {1}"
                        , command
                        , std::strerror (EINVAL)
                        )
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
