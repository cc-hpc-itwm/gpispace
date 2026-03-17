#include <gspc/util/refcounted_set.hpp>

#include <gspc/testing/random.hpp>
#include <gspc/testing/test_case.hpp>

#include <boost/test/unit_test.hpp>

#include <set>
#include <string>

GSPC_TESTING_TEMPLATED_CASE (refcounted_set, int, std::string)
{
  gspc::util::refcounted_set<T> s;

  gspc::testing::unique_random<T> random;

  T const a {random()};
  T const b {random()};

  BOOST_REQUIRE_EQUAL (s.contains (a), false);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 0);

  s.emplace (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 1);

  s.erase (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), false);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 0);

  s.emplace (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 1);

  s.emplace (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 1);

  s.erase (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 1);

  s.erase (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), false);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 0);

  s.emplace (a);
  s.emplace (a);
  s.emplace (b);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), true);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 2);

  s.emplace (b);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), true);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 2);

  s.erase (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), true);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 2);

  s.erase (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), false);
  BOOST_REQUIRE_EQUAL (s.contains (b), true);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 1);

  s.erase (b);
  BOOST_REQUIRE_EQUAL (s.contains (a), false);
  BOOST_REQUIRE_EQUAL (s.contains (b), true);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 1);

  s.erase (b);
  BOOST_REQUIRE_EQUAL (s.contains (a), false);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 0);
}

BOOST_AUTO_TEST_CASE (emplace_works_with_temporaries)
{
  gspc::util::refcounted_set<std::string> s;

  std::string const content (gspc::testing::random<std::string>{}());
  //! \note explicitly make temporary
  s.emplace (std::string (content));
  BOOST_REQUIRE (s.contains (content));
  s.erase (content);
  BOOST_REQUIRE (!s.contains (content));
}
