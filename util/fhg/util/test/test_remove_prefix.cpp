// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE RemovePrefix
#include <boost/test/unit_test.hpp>

#include <fhg/util/remove_prefix.hpp>

namespace
{
  const std::string word ("ababab");
}

BOOST_AUTO_TEST_CASE (different_length_prefixes)
{
  {
    const std::string prefix ("");

    BOOST_REQUIRE_EQUAL (fhg::util::remove_prefix (prefix, word), "ababab");
  }

  {
    const std::string prefix ("a");

    BOOST_REQUIRE_EQUAL (fhg::util::remove_prefix (prefix, word), "babab");
  }

  {
    const std::string prefix ("abab");

    BOOST_REQUIRE_EQUAL (fhg::util::remove_prefix (prefix, word), "ab");
  }

  {
    const std::string prefix ("ababab");

    BOOST_REQUIRE_EQUAL (fhg::util::remove_prefix (prefix, word), "");
  }
}

BOOST_AUTO_TEST_CASE (longer_prefix_than_input)
{
  const std::string prefix ("abababab");

  try
  {
    fhg::util::remove_prefix (prefix, word);

    BOOST_FAIL ("should throw");
  }
  catch (const fhg::util::remove_prefix_failed& f)
  {
    BOOST_REQUIRE_EQUAL (f.word(), "");
    BOOST_REQUIRE_EQUAL (f.prefix(), "ab");
  }
  catch (...)
  {
    BOOST_FAIL ("should throw fhg::util::remove_prefix_failed");
  }
}

BOOST_AUTO_TEST_CASE (non_matched_prefix)
{
  const std::string prefix ("A");

  try
  {
    fhg::util::remove_prefix (prefix, word);

    BOOST_FAIL ("should throw");
  }
  catch (const fhg::util::remove_prefix_failed& f)
  {
    BOOST_REQUIRE_EQUAL (f.word(), "ababab");
    BOOST_REQUIRE_EQUAL (f.prefix(), "A");
  }
  catch (...)
  {
    BOOST_FAIL ("should throw fhg::util::remove_prefix_failed");
  }
}
