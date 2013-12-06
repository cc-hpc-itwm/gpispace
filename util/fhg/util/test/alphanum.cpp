// bernd.loerwald@itwm.fraunhofer.de

#define BOOST_TEST_MODULE alphanum
#include <boost/test/unit_test.hpp>

#include <fhg/util/alphanum.hpp>

#include <set>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (compare_less)
{
  fhg::util::alphanum::less pred;

  BOOST_REQUIRE_EQUAL (pred ("", "_"), true);
  BOOST_REQUIRE_EQUAL (pred ("", ""), false);
  BOOST_REQUIRE_EQUAL (pred ("_", ""), false);

  BOOST_REQUIRE_EQUAL (pred ("2", "10"), true);
  BOOST_REQUIRE_EQUAL (pred ("2", "2"), false);
  BOOST_REQUIRE_EQUAL (pred ("10", "2"), false);

  BOOST_REQUIRE_EQUAL (pred ("foo2", "foo10"), true);
  BOOST_REQUIRE_EQUAL (pred ("foo2", "foo2"), false);
  BOOST_REQUIRE_EQUAL (pred ("foo2", "bar10"), false);

  BOOST_REQUIRE_EQUAL (pred ("a", "a"), false);
  BOOST_REQUIRE_EQUAL (pred ("a", "b"), true);
  BOOST_REQUIRE_EQUAL (pred ("aaaaa", "b"), true);
  BOOST_REQUIRE_EQUAL (pred ("a", "bbbbb"), true);
  BOOST_REQUIRE_EQUAL (pred ("aaaaa", "bbbbb"), true);
  BOOST_REQUIRE_EQUAL (pred ("aaaaa", "aaaaa"), false);
  BOOST_REQUIRE_EQUAL (pred ("aaaaa", "aaaa"), false);
  BOOST_REQUIRE_EQUAL (pred ("aaaa", "aaaaa"), true);
}

BOOST_AUTO_TEST_CASE (compare_less_equal)
{
  fhg::util::alphanum::less_equal pred;

  BOOST_REQUIRE_EQUAL (pred ("", "_"), true);
  BOOST_REQUIRE_EQUAL (pred ("", ""), true);
  BOOST_REQUIRE_EQUAL (pred ("_", ""), false);

  BOOST_REQUIRE_EQUAL (pred ("2", "10"), true);
  BOOST_REQUIRE_EQUAL (pred ("2", "2"), true);
  BOOST_REQUIRE_EQUAL (pred ("10", "2"), false);

  BOOST_REQUIRE_EQUAL (pred ("foo2", "foo10"), true);
  BOOST_REQUIRE_EQUAL (pred ("foo2", "foo2"), true);
  BOOST_REQUIRE_EQUAL (pred ("foo2", "bar10"), false);

  BOOST_REQUIRE_EQUAL (pred ("a", "a"), true);
  BOOST_REQUIRE_EQUAL (pred ("a", "b"), true);
  BOOST_REQUIRE_EQUAL (pred ("aaaaa", "b"), true);
  BOOST_REQUIRE_EQUAL (pred ("a", "bbbbb"), true);
  BOOST_REQUIRE_EQUAL (pred ("aaaaa", "bbbbb"), true);
  BOOST_REQUIRE_EQUAL (pred ("aaaaa", "aaaaa"), true);
  BOOST_REQUIRE_EQUAL (pred ("aaaaa", "aaaa"), false);
  BOOST_REQUIRE_EQUAL (pred ("aaaa", "aaaaa"), true);
}

BOOST_AUTO_TEST_CASE (sort_less_equal)
{
  std::vector<std::string> data;

  data.push_back ("2");
  data.push_back ("10");
  data.push_back ("2");
  data.push_back ("2");
  data.push_back ("10");
  data.push_back ("2");

  data.push_back ("foo2");
  data.push_back ("foo10");
  data.push_back ("foo2");
  data.push_back ("foo2");
  data.push_back ("foo2");
  data.push_back ("bar10");

  data.push_back ("a");
  data.push_back ("a");
  data.push_back ("a");
  data.push_back ("b");
  data.push_back ("aaaaa");
  data.push_back ("b");
  data.push_back ("a");
  data.push_back ("bbbbb");
  data.push_back ("aaaaa");
  data.push_back ("bbbbb");
  data.push_back ("aaaaa");
  data.push_back ("aaaaa");
  data.push_back ("aaaaa");
  data.push_back ("aaaa");
  data.push_back ("aaaa");
  data.push_back ("aaaaa");

  std::sort (data.begin(), data.end(), fhg::util::alphanum::less_equal());

  {
    std::vector<std::string> reference;

    reference.push_back ("2");
    reference.push_back ("2");
    reference.push_back ("2");
    reference.push_back ("2");
    reference.push_back ("10");
    reference.push_back ("10");
    reference.push_back ("a");
    reference.push_back ("a");
    reference.push_back ("a");
    reference.push_back ("a");
    reference.push_back ("aaaa");
    reference.push_back ("aaaa");
    reference.push_back ("aaaaa");
    reference.push_back ("aaaaa");
    reference.push_back ("aaaaa");
    reference.push_back ("aaaaa");
    reference.push_back ("aaaaa");
    reference.push_back ("aaaaa");
    reference.push_back ("b");
    reference.push_back ("b");
    reference.push_back ("bar10");
    reference.push_back ("bbbbb");
    reference.push_back ("bbbbb");
    reference.push_back ("foo2");
    reference.push_back ("foo2");
    reference.push_back ("foo2");
    reference.push_back ("foo2");
    reference.push_back ("foo10");

    BOOST_REQUIRE_EQUAL_COLLECTIONS
      (data.begin(), data.end(), reference.begin(), reference.end());
  }
}

BOOST_AUTO_TEST_CASE (set_less)
{
  std::set<std::string, fhg::util::alphanum::less> data;

  data.insert ("2");
  data.insert ("10");
  data.insert ("2");
  data.insert ("2");
  data.insert ("10");
  data.insert ("2");

  data.insert ("foo2");
  data.insert ("foo10");
  data.insert ("foo2");
  data.insert ("foo2");
  data.insert ("foo2");
  data.insert ("bar10");

  data.insert ("a");
  data.insert ("a");
  data.insert ("a");
  data.insert ("b");
  data.insert ("aaaaa");
  data.insert ("b");
  data.insert ("a");
  data.insert ("bbbbb");
  data.insert ("aaaaa");
  data.insert ("bbbbb");
  data.insert ("aaaaa");
  data.insert ("aaaaa");
  data.insert ("aaaaa");
  data.insert ("aaaa");
  data.insert ("aaaa");
  data.insert ("aaaaa");

  {
    std::vector<std::string> reference;

    reference.push_back ("2");
    reference.push_back ("10");
    reference.push_back ("a");
    reference.push_back ("aaaa");
    reference.push_back ("aaaaa");
    reference.push_back ("b");
    reference.push_back ("bar10");
    reference.push_back ("bbbbb");
    reference.push_back ("foo2");
    reference.push_back ("foo10");

    BOOST_REQUIRE_EQUAL_COLLECTIONS
      (data.begin(), data.end(), reference.begin(), reference.end());
  }
}
