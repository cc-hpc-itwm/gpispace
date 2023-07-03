// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/read_file.hpp>
#include <util-generic/scoped_file_with_content.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/filesystem.hpp>

BOOST_AUTO_TEST_CASE (scoped_file_with_content)
{
  fhg::util::temporary_path const temporary_directory
    {::boost::filesystem::unique_path()};

  ::boost::filesystem::path const path
    {temporary_directory / ::boost::filesystem::unique_path()};

  BOOST_REQUIRE (!::boost::filesystem::exists (path));

  {
    std::string const content {fhg::util::testing::random<std::string>{}()};

    fhg::util::scoped_file_with_content const _ (path, content);

    BOOST_REQUIRE_EQUAL (fhg::util::read_file (path), content);
  }

  BOOST_REQUIRE (!::boost::filesystem::exists (path));
}
