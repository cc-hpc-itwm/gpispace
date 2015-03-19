// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE read_lines
#include <boost/test/unit_test.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/read_lines.hpp>

#include <util-generic/syscall.hpp>
#include <fhg/util/temporary_file.hpp>
#include <fhg/util/temporary_path.hpp>
#include <fhg/util/boost/test/printer/vector.hpp>
#include <fhg/util/boost/test/require_exception.hpp>

#include <test/shared_directory.hpp>

#include <fstream>
#include <functional>

namespace
{
  void verify ( boost::filesystem::path const& temporary_directory
              , std::string const& content
              , std::vector<std::string> const& expected_lines
              )
  {
    fhg::util::temporary_file const _
      ( temporary_directory
      / boost::filesystem::unique_path ("content-%%%%-%%%%-%%%%-%%%%")
      );

    BOOST_REQUIRE (!boost::filesystem::exists (_));

    {
      std::ofstream stream (boost::filesystem::path (_).string());
      stream << content;
    }

    BOOST_REQUIRE_EQUAL (fhg::util::read_lines (_), expected_lines);
  }
}

BOOST_AUTO_TEST_CASE (read_lines)
{
  boost::program_options::options_description options_description;
  options_description.add (test::options::shared_directory());

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
    ( boost::unit_test::framework::master_test_suite().argc
    , boost::unit_test::framework::master_test_suite().argv
    )
    . options (options_description).run()
    , vm
    );
  vm.notify();

  fhg::util::temporary_path const temporary_directory
    ( test::shared_directory (vm)
    / boost::filesystem::unique_path ("read_lines_%%%%-%%%%-%%%%-%%%%")
    );

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
  boost::filesystem::path const _
    (boost::filesystem::unique_path ("non-existing-path-%%%%-%%%%-%%%%-%%%%"));
  BOOST_REQUIRE (!boost::filesystem::exists (_));

  fhg::util::boost::test::require_exception<std::runtime_error>
    ( std::bind (fhg::util::read_lines, _)
    , boost::format ("could not open file '%1%' for reading: %2%")
      % _.string()
      % "No such file or directory"
    );
}

BOOST_AUTO_TEST_CASE (read_lines_check_throw_on_permission_denied)
{
  boost::program_options::options_description options_description;
  options_description.add (test::options::shared_directory());

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
    ( boost::unit_test::framework::master_test_suite().argc
    , boost::unit_test::framework::master_test_suite().argv
    )
    . options (options_description).run()
    , vm
    );
  vm.notify();

  boost::filesystem::path const _
    ( test::shared_directory (vm)
    / boost::filesystem::unique_path ("non-readable-%%%%-%%%%-%%%%-%%%%")
    );

  fhg::util::temporary_file const tmpfile (_);
  {
    std::ofstream (_.string());
    fhg::util::syscall::chmod (_.string().c_str(), 0);
  }
  BOOST_REQUIRE (boost::filesystem::exists (_));

  fhg::util::boost::test::require_exception<std::runtime_error>
    ( std::bind (fhg::util::read_lines, _)
    , boost::format ("could not open file '%1%' for reading: %2%")
    % _.string()
    % "Permission denied"
    );
}
