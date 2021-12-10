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

#include <fhgcom/address.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
namespace com
{
BOOST_AUTO_TEST_CASE ( test_address_translation_equal )
{
  BOOST_CHECK (  fhg::com::p2p::address_t (host_t {"name"}, port_t {1})
              == fhg::com::p2p::address_t (host_t {"name"}, port_t {1})
              );
}

BOOST_AUTO_TEST_CASE ( test_address_translation_unique )
{
  BOOST_CHECK (  fhg::com::p2p::address_t (host_t {"name"}, port_t {1})
              != fhg::com::p2p::address_t (host_t {"name"}, port_t {2})
              );
}
}
}
