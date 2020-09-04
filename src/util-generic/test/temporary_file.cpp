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

#include <util-generic/temporary_file.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/filesystem.hpp>

#include <fstream>

BOOST_AUTO_TEST_CASE (temporary_file)
{
  boost::filesystem::path const path ("temporary_file-temporary_file");

  BOOST_REQUIRE (!boost::filesystem::exists (path));

  {
    fhg::util::temporary_file const _ (path);

    std::ofstream const __ (path.string().c_str());

    BOOST_REQUIRE (boost::filesystem::exists (path));
  }

  BOOST_REQUIRE (!boost::filesystem::exists (path));
}
