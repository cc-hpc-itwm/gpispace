// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_value
#include <boost/test/unit_test.hpp>

#include <we/type/value.hpp>
#include <we/type/value/get.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/num.hpp>

#include <sstream>

namespace
{
  template<typename T>
  void test_show_and_read_showed (const T& x, const std::string& expected)
  {
    using pnet::type::value::value_type;
    using pnet::type::value::read;
    using fhg::util::parse::position;

    std::ostringstream oss;
    oss << value_type (x);
    BOOST_REQUIRE_EQUAL (expected, oss.str());
    const std::string inp (oss.str());
    position pos (inp);
    BOOST_REQUIRE (value_type (x) == read (pos));
    BOOST_REQUIRE (pos.end());
  }
}

BOOST_AUTO_TEST_CASE (num_type)
{
  using pnet::type::value::value_type;
  using fhg::util::num_type;

  BOOST_CHECK (value_type (23) == value_type (num_type (23)));
  BOOST_CHECK (value_type (23U) == value_type (num_type (23U)));
  BOOST_CHECK (value_type (23L) == value_type (num_type (23L)));
  BOOST_CHECK (value_type (23UL) == value_type (num_type (23UL)));
  BOOST_CHECK (value_type (23.0) == value_type (num_type (23.0)));
  BOOST_CHECK (value_type (23.0f) == value_type (num_type (23.0f)));
}

BOOST_AUTO_TEST_CASE (show_and_read_showed)
{
  using pnet::type::value::value_type;

  test_show_and_read_showed (we::type::literal::control(), "[]");
  test_show_and_read_showed (true, "true");
  test_show_and_read_showed (false, "false");
  test_show_and_read_showed (0, "0");
  test_show_and_read_showed (23, "23");
  test_show_and_read_showed (42L, "42L");
  test_show_and_read_showed (-3L, "-3L");
  test_show_and_read_showed (2718U, "2718U");
  test_show_and_read_showed (3141UL, "3141UL");
  test_show_and_read_showed (3.14159f, "3.14159f");
  test_show_and_read_showed (3e30f, "3.00000e+30f");
  test_show_and_read_showed (3.14159, "3.14159");
  test_show_and_read_showed (3e30, "3.00000e+30");
  test_show_and_read_showed (0.0, "0.00000");
  test_show_and_read_showed ('\0', std::string("'\0'", 3));
  test_show_and_read_showed ('a', "'a'");
  test_show_and_read_showed (std::string(), "\"\"");
  test_show_and_read_showed (std::string("foo"), "\"foo\"");
  test_show_and_read_showed (bitsetofint::type(), "{}");
  test_show_and_read_showed (bitsetofint::type().ins (0), "{ 1}");
  test_show_and_read_showed (bitsetofint::type().ins (0).ins (64), "{ 1 1}");
  test_show_and_read_showed (bytearray::type(), "y()");
  {
    std::list<value_type> l;
    test_show_and_read_showed (l, "list ()");
    l.push_back (value_type (int (3)));
    test_show_and_read_showed (l, "list (3)");
    l.push_back (value_type (long (3)));
    test_show_and_read_showed (l, "list (3, 3L)");
  }
  test_show_and_read_showed (std::vector<value_type>(), "vector ()");
  test_show_and_read_showed (std::set<value_type>(), "set {}");
  test_show_and_read_showed (std::map<value_type,value_type>(), "map []");
}

BOOST_AUTO_TEST_CASE (_read)
{
  using pnet::type::value::value_type;
  using pnet::type::value::read;

  BOOST_CHECK (value_type (true) == read ("true"));
  BOOST_CHECK (value_type (false) == read ("false"));
  BOOST_CHECK (value_type (we::type::literal::control()) == read ("[]"));
  BOOST_CHECK (value_type ('a') == read ("'a'"));
  BOOST_CHECK (value_type (std::string ("foo")) == read ("\"foo\""));
  BOOST_CHECK (value_type (std::string ("\"")) == read ("\"\\\"\""));
  BOOST_CHECK (value_type (std::string ("\\\"")) == read ("\"\\\\\\\"\""));
  BOOST_CHECK (value_type (std::string ("\"foo\"")) == read ("\"\\\"foo\\\"\""));

  BOOST_CHECK_THROW (read ("\"\\n\""), fhg::util::parse::error::expected);

  BOOST_CHECK (value_type (23) == read ("23"));
  BOOST_CHECK (value_type (23U) == read ("23U"));
  BOOST_CHECK (value_type (23L) == read ("23L"));
  BOOST_CHECK (value_type (23UL) == read ("23UL"));
  BOOST_CHECK (value_type (2.3f) == read ("2.3f"));
  BOOST_CHECK (value_type (2.3) == read ("2.3"));

  {
    bitsetofint::type bs;
    BOOST_CHECK (value_type (bs) == read ("{}"));
    bs.ins (0);
    BOOST_CHECK (value_type (bs) == read ("{ 1}"));
    bs.ins (64);
    BOOST_CHECK (value_type (bs) == read ("{ 1 1}"));
  }
  {
    bytearray::type ba;
    BOOST_CHECK (value_type (ba) == read ("y()"));
    ba.push_back (' ');
    BOOST_CHECK (value_type (ba) == read ("y( 32)"));
    ba.push_back ('A');
    BOOST_CHECK (value_type (ba) == read ("y( 32 65)"));
  }
  {
    std::map<value_type, value_type> m;

    BOOST_CHECK (value_type (m) == read ("map []"));

    m[value_type ('a')] = value_type (std::string ("foo"));
    m[value_type (std::string ("foo"))] = value_type ('a');

    BOOST_CHECK (value_type (m) == read ("map ['a'->\"foo\",\"foo\"->'a']"));
  }

  {
    std::map<std::string, value_type> m;

    BOOST_CHECK (value_type (m) == read ("struct []"));

    m["a"] = value_type ('a');
    m["foo"] = value_type ('b');

    BOOST_CHECK (value_type (m) == read ("struct [a:='a',foo:='b']"));
  }

  {
    std::vector<value_type> v;

    BOOST_CHECK (value_type (v) == read ("vector ()"));

    v.push_back ('a');
    v.push_back ('b');

    BOOST_CHECK (value_type (v) == read ("vector ('a', 'b')"));

    std::set<value_type> s;

    BOOST_CHECK (value_type (s) == read ("set{}"));

    s.insert (value_type ('a'));
    s.insert (v);

    BOOST_CHECK (value_type (s) == read ("set {'a', vector ('a', 'b')}"));
  }

  {
    std::vector<value_type> v;
    std::list<value_type> l;
    std::map<value_type, value_type> m;
    std::string input ("vector ()list( ) map[]");
    fhg::util::parse::position pos (input);

    BOOST_CHECK (value_type (v) == read (pos));
    BOOST_CHECK (value_type (l) == read (pos));
    BOOST_CHECK (value_type (m) == read (pos));
    BOOST_CHECK (pos.rest().empty());
 }
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
