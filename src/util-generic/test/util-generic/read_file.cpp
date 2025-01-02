// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

    BOOST_REQUIRE_EQUAL (fhg::util::read_file (path), content);
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
