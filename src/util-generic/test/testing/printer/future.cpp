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

#include <util-generic/test/testing/printer/require_printed_as.hpp>
#include <util-generic/testing/printer/future.hpp>

#include <boost/test/unit_test.hpp>

#include <future>

//! \note Yes, this test is dumb. It mainly is that way since it is
//! impossible to construct a scenario where std::future::wait_for
//! guaranteed returns deferred.
BOOST_AUTO_TEST_CASE (std_future_status_enum_is_printed)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("ready", std::future_status::ready);
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("timeout", std::future_status::timeout);
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("deferred", std::future_status::deferred);
}
