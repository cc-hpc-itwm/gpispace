// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_value
#include <boost/test/unit_test.hpp>

#include <we/type/value.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/get.hpp>
#include <we/type/value/poke.hpp>

#include <sstream>

namespace
{
  template<typename T>
  void test_show (const T& x, const std::string& expected)
  {
    std::ostringstream oss;
    oss << pnet::type::value::value_type (x);
    BOOST_REQUIRE_EQUAL (expected, oss.str());
  }
}

BOOST_AUTO_TEST_CASE (show)
{
  using pnet::type::value::value_type;

  test_show (we::type::literal::control(), "[]");
  test_show (true, "true");
  test_show (false, "false");
  test_show (0, "0");
  test_show (23, "23");
  test_show (42L, "42L");
  test_show (-3L, "-3L");
  test_show (2718U, "2718U");
  test_show (3141UL, "3141UL");
  test_show (3.1415926f, "3.14159f");
  test_show (3e30f, "3.00000e+30f");
  test_show (3.1415926, "3.14159");
  test_show (3e30, "3.00000e+30");
  test_show (0.0, "0.00000");
  test_show ('\0', std::string("'\0'", 3));
  test_show ('a', "'a'");
  test_show (std::string(), "\"\"");
  test_show (std::string("foo"), "\"foo\"");
  test_show (bitsetofint::type(), "{}");
  test_show (bitsetofint::type().ins (0), "{ 1}");
  test_show (bitsetofint::type().ins (0).ins (64), "{ 1 1}");
  test_show (bytearray::type(), "y()");
  {
    std::list<value_type> l;
    test_show (l, "list ()");
    l.push_back (value_type (int (3)));
    test_show (l, "list (3)");
    l.push_back (value_type (long (3)));
    test_show (l, "list (3, 3L)");
  }
  test_show (std::vector<value_type>(), "vector ()");
  test_show (std::set<value_type>(), "set {}");
  test_show (std::map<value_type,value_type>(), "map []");
}

BOOST_AUTO_TEST_CASE (get)
{
  using pnet::type::value::value_type;
  using pnet::type::value::get;

  const value_type s (std::string ("s"));

  BOOST_REQUIRE_EQUAL (boost::get<std::string> (s), "s");
  BOOST_REQUIRE_EQUAL (boost::get<const std::string&> (s), "s");
  BOOST_REQUIRE_THROW (boost::get<std::string&> (s), boost::bad_get);

  value_type i (int (0));

  BOOST_REQUIRE_EQUAL (boost::get<const int&> (i), 0);

  boost::get<int&> (i) = 1;

  BOOST_REQUIRE_EQUAL (boost::get<const int&> (i), 1);

  value_type l1;
  {
    std::list<value_type> _l1;
    _l1.push_back (i);
    _l1.push_back (s);
    _l1.push_back (3L);
    l1 = _l1;
  }

  value_type m1 (pnet::type::value::empty());
  boost::get<std::map<std::string, value_type>&> (m1)["i"] = i;
  boost::get<std::map<std::string, value_type>&> (m1)["s"] = s;
  boost::get<std::map<std::string, value_type>&> (m1)["l1"] = l1;

  std::list<value_type> _l2;
  _l2.push_back (l1);
  _l2.push_back (m1);
  const value_type l2 (_l2);

  std::map<value_type, value_type> _mv;
  _mv[i] = s;
  _mv[m1] = l2;
  const value_type mv (_mv);

  std::set<value_type> _set;
  _set.insert (i);
  _set.insert (i);
  _set.insert (s);
  _set.insert (l1);
  const value_type set (_set);

  std::map<std::string, value_type> _m2;
  _m2["m1"] = m1;
  _m2["l2"] = l2;
  _m2["mv"] = mv;
  _m2["set"] = set;
  const value_type m2 (_m2);

  {
    std::list<std::string> keys;
    keys.push_back ("set");
    BOOST_CHECK (get (keys, m2));
    BOOST_CHECK (*get (keys, m2) == set);
    // BOOST_REQUIRE_EQUAL (*get (keys, m2), set);
    // | Does not compile. Why?
  }
  {
    std::list<std::string> keys;
    keys.push_back ("m1");
    keys.push_back ("l1");
    BOOST_CHECK (get (keys, m2));
    BOOST_CHECK (*get (keys, m2) == l1);
  }
}

BOOST_AUTO_TEST_CASE (get_ref)
{
  using pnet::type::value::value_type;
  using pnet::type::value::poke;
  using pnet::type::value::get;
  using pnet::type::value::get_ref;

  const value_type l = std::list<value_type>();

  std::list<std::string> keys;
  keys.push_back ("l");

  value_type m;
  poke (keys, m, l);

  BOOST_CHECK (get (keys, m));

  {
    const std::list<value_type>& g
      (boost::get<const std::list<value_type>&> (*get (keys, m)));

    BOOST_CHECK (g.empty());
  }

  BOOST_CHECK (get_ref (keys, m));

  {
    std::list<value_type>& r
      (boost::get<std::list<value_type>&> (*get_ref (keys, m)));

    BOOST_CHECK (r.empty());

    r.push_back (19);
  }

  {
    const std::list<value_type>& g
      (boost::get<const std::list<value_type>&> (*get (keys, m)));

    BOOST_CHECK (g.size() == 1);
    BOOST_CHECK (*g.begin() == value_type (19));
  }
}

BOOST_AUTO_TEST_CASE (poke)
{
  using pnet::type::value::value_type;
  using pnet::type::value::poke;
  using pnet::type::value::get;

  value_type v;

  const value_type s (std::string ("s"));
  const value_type i (int (1));

  {
    std::list<std::string> keys;
    keys.push_back ("s");
    poke (keys, v, s);

    BOOST_CHECK (get (keys, v));
    BOOST_CHECK (*get (keys, v) == s);
  }
  {
    std::list<std::string> keys;
    keys.push_back ("i");
    poke (keys, v, i);
    BOOST_CHECK (get (keys, v));
    BOOST_CHECK (*get (keys, v) == i);

    keys.push_back ("i");
    poke (keys, v, i);
    BOOST_CHECK (get (keys, v));
    BOOST_CHECK (*get (keys, v) == i);

    keys.pop_back();
    BOOST_CHECK (get (keys, v));
    BOOST_CHECK (get (keys, *get (keys, v)));
    BOOST_CHECK (*get (keys, *get (keys, v)) == i);
  }
}
