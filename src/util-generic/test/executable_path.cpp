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

#include <util-generic/executable_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/filesystem/operations.hpp>

BOOST_AUTO_TEST_CASE (executable_path)
{
  BOOST_REQUIRE_EQUAL
    ( boost::filesystem::canonical
      (boost::unit_test::framework::master_test_suite().argv[0])
    , fhg::util::executable_path()
    );
}

static int symbol_in_this_binary;

BOOST_AUTO_TEST_CASE (executable_path_symbol)
{
  BOOST_REQUIRE_EQUAL
    ( boost::filesystem::canonical
      (boost::unit_test::framework::master_test_suite().argv[0])
    , fhg::util::executable_path (&symbol_in_this_binary)
    );
}
