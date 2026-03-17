#include <boost/test/unit_test.hpp>

#include <gspc/util/read_lines.hpp>
#include <gspc/util/syscall.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/vector.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/testing/unique_path.hpp>
#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>

#include <fmt/core.h>
#include <fstream>
#include <functional>
#include <utility>

namespace
{
  void verify ( std::filesystem::path const& temporary_directory
              , std::string const& content
              , std::vector<std::string> const& expected_lines
              )
  {
    gspc::util::temporary_file const _
      ( temporary_directory
      / gspc::testing::unique_path()
      );

    BOOST_REQUIRE (!std::filesystem::exists (_));

    std::ofstream {std::filesystem::path {_}} << content;

    BOOST_REQUIRE_EQUAL (gspc::util::read_lines (_), expected_lines);
  }
}

BOOST_AUTO_TEST_CASE (read_lines)
{
  gspc::testing::temporary_path const temporary_directory;

  verify (temporary_directory, "", {});
  verify (temporary_directory, "\n", {""});
  verify (temporary_directory, "foo", {"foo"});
  verify (temporary_directory, "foo\n", {"foo"});
  verify (temporary_directory, "foo\n\n", {"foo", ""});
  verify (temporary_directory, "foo\nbar", {"foo", "bar"});
  verify (temporary_directory, "foo bar", {"foo bar"});
  verify (temporary_directory, "foo bar\nbaz faz", {"foo bar", "baz faz"});
}

BOOST_AUTO_TEST_CASE (read_lines_check_throw_on_no_such_file_or_directory)
{
  std::filesystem::path const _
    {gspc::testing::unique_path()};
  BOOST_REQUIRE (!std::filesystem::exists (_));

  gspc::testing::require_exception
    ( [&]
      {
        return gspc::util::read_lines (_);
      }
    , std::runtime_error
        { fmt::format ( "could not open file '{}' for reading: {}"
                      , _
                      , "No such file or directory"
                      )
        }
    );
}

BOOST_AUTO_TEST_CASE (read_lines_check_throw_on_permission_denied)
{
  if (gspc::util::syscall::getuid() == uid_t {0})
  {
    return;
  }

  std::filesystem::path const _
    {gspc::testing::unique_path()};

  gspc::util::temporary_file const tmpfile (_);
  {
    std::ignore = std::ofstream (_);
    gspc::util::syscall::chmod (_.c_str(), 0);
  }
  BOOST_REQUIRE (std::filesystem::exists (_));

  gspc::testing::require_exception
    ( [&]
      {
        return gspc::util::read_lines (_);
      }
    , std::runtime_error
        { fmt::format ( "could not open file '{}' for reading: {}"
                      , _
                      , "Permission denied"
                      )
        }
    );
}
