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

#include <fhgcom/message.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

#include <string>

BOOST_AUTO_TEST_CASE ( test_message_constr_default )
{
  BOOST_REQUIRE_EQUAL (fhg::com::message_t().size (), 0);
}

BOOST_AUTO_TEST_CASE ( test_message_constr_with_length )
{
  BOOST_REQUIRE_EQUAL (fhg::com::message_t (10).size(), 10);
}

BOOST_AUTO_TEST_CASE ( test_message_constr_with_data )
{
  const char * data = "Hello World!";
  BOOST_REQUIRE_EQUAL
    (fhg::com::message_t (data, strlen (data) + 1).size(), strlen (data) + 1);
}

BOOST_AUTO_TEST_CASE ( test_message_constr_copy )
{
  fhg::com::message_t msg1 ( "abc", 4 );
  fhg::com::message_t msg2 ( msg1 );
  BOOST_REQUIRE_EQUAL ( msg1.size(), 4 );
  BOOST_REQUIRE_EQUAL ( std::string(msg1.buf()), "abc" );

  BOOST_REQUIRE_EQUAL ( msg2.size(), 4 );
  BOOST_REQUIRE_EQUAL ( std::string(msg2.buf()), "abc" );
}

BOOST_AUTO_TEST_CASE ( test_message_assignment )
{
  fhg::com::message_t msg1;
  fhg::com::message_t msg2 ( 42 );
  msg1 = msg2;
  BOOST_REQUIRE_EQUAL ( msg1.size(), 42 );
}
