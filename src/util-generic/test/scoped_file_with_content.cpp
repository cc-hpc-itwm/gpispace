// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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
#include <util-generic/scoped_file_with_content.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/filesystem.hpp>

BOOST_AUTO_TEST_CASE (scoped_file_with_content)
{
  fhg::util::temporary_path const temporary_directory
    {boost::filesystem::unique_path()};

  boost::filesystem::path const path
    {temporary_directory / boost::filesystem::unique_path()};

  BOOST_REQUIRE (!boost::filesystem::exists (path));

  {
    std::string const content {fhg::util::testing::random<std::string>{}()};

    fhg::util::scoped_file_with_content const _ (path, content);

    BOOST_REQUIRE_EQUAL (fhg::util::read_file (path), content);
  }

  BOOST_REQUIRE (!boost::filesystem::exists (path));
}
