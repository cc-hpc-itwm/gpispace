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

#include <util-generic/read_lines.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <fstream>
#include <functional>

namespace
{
  void verify ( ::boost::filesystem::path const& temporary_directory
              , std::string const& content
              , std::vector<std::string> const& expected_lines
              )
  {
    fhg::util::temporary_file const _
      ( temporary_directory
      / ::boost::filesystem::unique_path ("content-%%%%-%%%%-%%%%-%%%%")
      );

    BOOST_REQUIRE (!::boost::filesystem::exists (_));

    {
      std::ofstream stream (::boost::filesystem::path (_).string());
      stream << content;
    }

    BOOST_REQUIRE_EQUAL (fhg::util::read_lines (_), expected_lines);
  }
}

BOOST_AUTO_TEST_CASE (read_lines)
{
  fhg::util::temporary_path const temporary_directory
    (::boost::filesystem::unique_path ("read_lines_%%%%-%%%%-%%%%-%%%%"));

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
  ::boost::filesystem::path const _
    (::boost::filesystem::unique_path ("non-existing-path-%%%%-%%%%-%%%%-%%%%"));
  BOOST_REQUIRE (!::boost::filesystem::exists (_));

  fhg::util::testing::require_exception
    ( std::bind (fhg::util::read_lines, _)
    , std::runtime_error
        ( ( ::boost::format ("could not open file '%1%' for reading: %2%")
          % _.string()
          % "No such file or directory"
          ).str()
        )
    );
}

BOOST_AUTO_TEST_CASE (read_lines_check_throw_on_permission_denied)
{
  ::boost::filesystem::path const _
    (::boost::filesystem::unique_path ("non-readable-%%%%-%%%%-%%%%-%%%%"));

  fhg::util::temporary_file const tmpfile (_);
  {
    std::ofstream (_.string());
    fhg::util::syscall::chmod (_.string().c_str(), 0);
  }
  BOOST_REQUIRE (::boost::filesystem::exists (_));

  fhg::util::testing::require_exception
    ( std::bind (fhg::util::read_lines, _)
    , std::runtime_error
        ( ( ::boost::format ("could not open file '%1%' for reading: %2%")
          % _.string()
          % "Permission denied"
          ).str()
        )
    );
}
