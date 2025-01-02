// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID ({}, ::boost::filesystem::path);
}

BOOST_AUTO_TEST_CASE (current_path)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ({::boost::filesystem::current_path()}, ::boost::filesystem::path);
}

BOOST_AUTO_TEST_CASE (relative)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ({"this/is/a/relative/path"}, ::boost::filesystem::path);
}

BOOST_AUTO_TEST_CASE (absolute)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ({"/this/is/an/absolute/path"}, ::boost::filesystem::path);
}
