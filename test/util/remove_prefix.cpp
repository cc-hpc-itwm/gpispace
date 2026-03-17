// Copyright (C) 2012,2015-2016,2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/util/remove_prefix.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>

namespace
{
  const std::string word ("ababab");
}

BOOST_AUTO_TEST_CASE (different_length_prefixes)
{
  {
    const std::string prefix ("");

    BOOST_REQUIRE_EQUAL (gspc::util::remove_prefix (prefix, word), "ababab");
  }

  {
    const std::string prefix ("a");

    BOOST_REQUIRE_EQUAL (gspc::util::remove_prefix (prefix, word), "babab");
  }

  {
    const std::string prefix ("abab");

    BOOST_REQUIRE_EQUAL (gspc::util::remove_prefix (prefix, word), "ab");
  }

  {
    const std::string prefix ("ababab");

    BOOST_REQUIRE_EQUAL (gspc::util::remove_prefix (prefix, word), "");
  }
}

BOOST_AUTO_TEST_CASE (longer_prefix_than_input)
{
  const std::string prefix ("abababab");

  try
  {
    gspc::util::remove_prefix (prefix, word);

    BOOST_FAIL ("should throw");
  }
  catch (gspc::util::remove_prefix_failed const& f)
  {
    BOOST_REQUIRE_EQUAL (f.word(), "");
    BOOST_REQUIRE_EQUAL (f.prefix(), "ab");
  }
}

BOOST_AUTO_TEST_CASE (non_matched_prefix)
{
  const std::string prefix ("A");

  try
  {
    gspc::util::remove_prefix (prefix, word);

    BOOST_FAIL ("should throw");
  }
  catch (gspc::util::remove_prefix_failed const& f)
  {
    BOOST_REQUIRE_EQUAL (f.word(), "ababab");
    BOOST_REQUIRE_EQUAL (f.prefix(), "A");
  }
}
