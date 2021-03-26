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

#include <fhgcom/header.hpp>
#include <fhgcom/tests/address_printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE ( test_address )
{
  BOOST_CHECK_EQUAL (16ul, sizeof (fhg::com::p2p::address_t));

  BOOST_CHECK_EQUAL (40ul, sizeof (fhg::com::p2p::header_t));
}

BOOST_AUTO_TEST_CASE ( test_address_translation_equal )
{
  BOOST_CHECK_EQUAL ( fhg::com::p2p::address_t ("name-1")
                    , fhg::com::p2p::address_t ("name-1")
                    );
}

BOOST_AUTO_TEST_CASE ( test_address_translation_unique )
{
  BOOST_CHECK_NE ( fhg::com::p2p::address_t ("name-1")
                 , fhg::com::p2p::address_t ("name-2")
                 );
}
