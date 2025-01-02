// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/printer/future.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

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
