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

#include <boost/test/unit_test.hpp>

#include <util-generic/read_file.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/random.hpp>

#include <fstream>

namespace
{
  void test (std::string const& content)
  {
    ::boost::filesystem::path const path ("temporary_file-read_file");

    BOOST_REQUIRE (!::boost::filesystem::exists (path));

    fhg::util::temporary_file const _ (path);

    std::ofstream (path.string().c_str()) << content;

    BOOST_REQUIRE_EQUAL (fhg::util::read_file (path.string()), content);
  }
}

BOOST_AUTO_TEST_CASE (read_file)
{
  test ("");
  test ("foo");
  test ("foo\n");
  test ("foo\nbar");
  test ("foo bar");
  test ("foo bar\nbaz faz\n");
}

BOOST_AUTO_TEST_CASE (read_file_as)
{
  ::boost::filesystem::path const path ("temporary_file-read_file");

  BOOST_REQUIRE (!::boost::filesystem::exists (path));

  fhg::util::temporary_file const _ (path);

  std::ofstream (path.string()) << path;

  BOOST_REQUIRE_EQUAL
    (fhg::util::read_file_as<::boost::filesystem::path> (path), path);
}

BOOST_AUTO_TEST_CASE (read_into_vector_of_char)
{
  ::boost::filesystem::path const path ("temporary_file-read_file");

  BOOST_REQUIRE (!::boost::filesystem::exists (path));

  fhg::util::temporary_file const _ (path);

  std::vector<char> const content
    {fhg::util::testing::randoms<std::vector<char>> (1 << 20)};

  std::ofstream (path.string()) << std::string (content.begin(), content.end());

  std::vector<char> const read {fhg::util::read_file<std::vector<char>> (path)};

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (content.begin(), content.end(), read.begin(), read.end());
}
