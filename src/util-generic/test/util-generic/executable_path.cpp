// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/executable_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/filesystem/operations.hpp>

BOOST_AUTO_TEST_CASE (executable_path)
{
  BOOST_REQUIRE_EQUAL
    ( ::boost::filesystem::canonical
      (::boost::unit_test::framework::master_test_suite().argv[0])
    , fhg::util::executable_path()
    );
}

static int symbol_in_this_binary;

BOOST_AUTO_TEST_CASE (executable_path_symbol)
{
  BOOST_REQUIRE_EQUAL
    ( ::boost::filesystem::canonical
      (::boost::unit_test::framework::master_test_suite().argv[0])
    , fhg::util::executable_path (&symbol_in_this_binary)
    );
}
