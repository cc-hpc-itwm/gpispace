#include <boost/test/unit_test.hpp>

#include <gspc/util/system.hpp>

#include <gspc/util/read_file.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/testing/unique_path.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <cerrno>
#include <cstring>
#include <fmt/core.h>
#include <stdexcept>
#include <string>


  namespace gspc::util
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
      auto const path
        {gspc::testing::unique_path()};
      temporary_file const _temp (path);

      auto const content ( gspc::testing::random<std::string>{}
                             (gspc::testing::random<std::string>::identifier{})
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
      gspc::testing::require_exception
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
      auto const description ( gspc::testing::random<std::string>{}
                                 (gspc::testing::random<std::string>::identifier{})
                             );

      gspc::testing::require_exception
        ( [this, description]
          {
            system<std::logic_error> (description, command());
          }
        , gspc::testing::make_nested
          ( std::logic_error ("Could not " + description)
          , einval (command())
          )
        );
    }
  }
