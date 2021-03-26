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

#include <xml/parse/error.hpp>
#include <parser_fixture.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_FIXTURE_TEST_CASE (different_directions, parser_fixture)
{
  BOOST_REQUIRE_NO_THROW (parse ("connect_in_out_same.xpnet"));
  BOOST_REQUIRE_NO_THROW (parse ("connect_read_out_same.xpnet"));
}

BOOST_FIXTURE_TEST_CASE (in_and_read, parser_fixture)
{
  BOOST_REQUIRE_THROW ( parse ("connect_in_read_same.xpnet")
                      , ::xml::parse::error::duplicate_connect
                      );
}

BOOST_FIXTURE_TEST_CASE (one_direction_twice, parser_fixture)
{
  BOOST_REQUIRE_THROW ( parse ("connect_in_in_same.xpnet")
                      , ::xml::parse::error::duplicate_connect
                      );
  BOOST_REQUIRE_THROW ( parse ("connect_read_read_same.xpnet")
                      , ::xml::parse::error::duplicate_connect
                      );
  BOOST_REQUIRE_THROW ( parse ("connect_out_out_same.xpnet")
                      , ::xml::parse::error::duplicate_connect
                      );
}
