// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/serialization/boost/blank.hpp>
#include <util-generic/serialization/trivial.hpp>
#include <util-generic/testing/require_compiletime.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (is_trivial)
{
  FHG_UTIL_TESTING_COMPILETIME_REQUIRE_EQUAL
    (fhg::util::serialization::is_trivially_serializable<::boost::blank>{}, true);
}

BOOST_AUTO_TEST_CASE (empty)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID ({}, ::boost::blank);
}
