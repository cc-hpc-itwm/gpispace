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

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <sdpa/test/sdpa/UNSAFE_thread_event.hpp>

BOOST_AUTO_TEST_CASE ( signal_event )
{
  using namespace fhg::util::thread;
  typedef UNSAFE_event<int> my_event;

  my_event e;

  e.notify (41);
  BOOST_REQUIRE_EQUAL (41, e.wait());
}
