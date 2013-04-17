// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_value
#include <boost/test/unit_test.hpp>

#include <we/type/value.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/require_type.hpp>
#include <we/type/value/exception.hpp>

#include <we/type/value/signature/name.hpp>
#include <we/type/value/signature/name_of.hpp>
#include <we/type/value/signature/of_type.hpp>
#include <we/type/value/signature/show.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/num.hpp>

#include <sstream>

using pnet::type::value::structured_type;

namespace
{
  template<typename T>
  void test_show_and_read_showed ( const T& x
                                 , const std::string& expected_show
                                 , const std::string& expected_signature
                                 )
  {
    using pnet::type::value::value_type;
    using pnet::type::value::read;
    using pnet::type::value::as_signature;
    using fhg::util::parse::position;

    {
      std::ostringstream oss;
      oss << value_type (x);
      BOOST_CHECK_EQUAL (expected_show, oss.str());
      const std::string inp (oss.str());
      position pos (inp);
      BOOST_CHECK (value_type (x) == read (pos));
      BOOST_CHECK (pos.end());
    }

    {
      std::ostringstream oss;
      oss << as_signature (value_type (x));
      BOOST_CHECK_EQUAL ("signature " + expected_signature, oss.str());
    }
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

  test_show_and_read_showed (we::type::literal::control(), "[]", "control");
  test_show_and_read_showed (true, "true", "bool");
  test_show_and_read_showed (false, "false", "bool");
  test_show_and_read_showed (0, "0", "int");
  test_show_and_read_showed (23, "23", "int");
  test_show_and_read_showed (42L, "42L", "long");
  test_show_and_read_showed (-3L, "-3L", "long");
  test_show_and_read_showed (2718U, "2718U", "unsigned int");
  test_show_and_read_showed (3141UL, "3141UL", "unsigned long");
  test_show_and_read_showed (3.14159f, "3.14159f", "float");
  test_show_and_read_showed (3e30f, "3.00000e+30f", "float");
  test_show_and_read_showed (3.14159, "3.14159", "double");
  test_show_and_read_showed (3e30, "3.00000e+30", "double");
  test_show_and_read_showed (0.0, "0.00000", "double");
  test_show_and_read_showed ('\0', std::string("'\0'", 3), "char");
  test_show_and_read_showed ('a', "'a'", "char");
  test_show_and_read_showed (std::string(), "\"\"", "string");
  test_show_and_read_showed (std::string("foo"), "\"foo\"", "string");
  test_show_and_read_showed (bitsetofint::type(), "{}", "bitset");
  test_show_and_read_showed (bitsetofint::type().ins (0), "{ 1}", "bitset");
  test_show_and_read_showed (bitsetofint::type().ins (0).ins (64), "{ 1 1}", "bitset");
  test_show_and_read_showed (bytearray::type(), "y()", "bytearray");
  {
    std::list<value_type> l;
    test_show_and_read_showed (l, "list ()", "list <>");
    l.push_back (value_type (int (3)));
    test_show_and_read_showed (l, "list (3)", "list <int>");
    l.push_back (value_type (long (3)));
    test_show_and_read_showed (l, "list (3, 3L)", "list <int|long>");
  }
  {
    std::vector<value_type> v;
    test_show_and_read_showed (v, "vector ()", "vector <>");
    v.push_back (std::string ("foo"));
    v.push_back (3.141);
    v.push_back (3.141);
    v.push_back (3.141f);
    test_show_and_read_showed
      (v, "vector (\"foo\", 3.14100, 3.14100, 3.14100f)"
      , "vector <string|double|double|float>"
      );
  }
  {
    std::set<value_type> s;
    test_show_and_read_showed (s, "set {}", "set <>");
    s.insert (std::string ("foo"));
    s.insert (3.141);
    s.insert (3.141);
    s.insert (3.141f);
    test_show_and_read_showed (s, "set {3.14100f, 3.14100, \"foo\"}"
                              , "set <float|double|string>"
                              );
  }
  {
    std::map<value_type, value_type> m;
    test_show_and_read_showed (m, "map []", "map <>");
    m[std::string ("foo")] = 314U;
    m[14] = 14;
    m[std::map<value_type,value_type>()] = 14;
    test_show_and_read_showed
      (m, "map [14 -> 14, \"foo\" -> 314U, map [] -> 14]"
      , "map <int -> int|string -> unsigned int|map <> -> int>"
      );
  }
  {
    structured_type m;
    test_show_and_read_showed (m, "struct []", "struct []");
    m["foo"] = 314U;
    m["bar"] = 14UL;
    m["baz"] = std::list<value_type>();
    test_show_and_read_showed
      (m, "struct [bar := 14UL, baz := list (), foo := 314U]"
      , "struct [bar :: unsigned long, baz :: list <>, foo :: unsigned int]"
      );
  }
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
    structured_type m;

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
    BOOST_CHECK (pos.end());
 }
}

BOOST_AUTO_TEST_CASE (peek)
{
  using pnet::type::value::value_type;
  using pnet::type::value::peek;

  const value_type s (std::string ("s"));

  BOOST_CHECK_EQUAL (boost::get<std::string> (s), "s");
  BOOST_CHECK_EQUAL (boost::get<const std::string&> (s), "s");
  BOOST_CHECK_THROW (boost::get<std::string&> (s), boost::bad_get);

  value_type i (int (0));

  BOOST_CHECK_EQUAL (boost::get<const int&> (i), 0);

  boost::get<int&> (i) = 1;

  BOOST_CHECK_EQUAL (boost::get<const int&> (i), 1);

  value_type l1;
  {
    std::list<value_type> _l1;
    _l1.push_back (i);
    _l1.push_back (s);
    _l1.push_back (3L);
    l1 = _l1;
  }

  value_type m1 = structured_type();
  boost::get<structured_type&> (m1)["i"] = i;
  boost::get<structured_type&> (m1)["s"] = s;
  boost::get<structured_type&> (m1)["l1"] = l1;

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

  structured_type _m2;
  _m2["m1"] = m1;
  _m2["l2"] = l2;
  _m2["mv"] = mv;
  _m2["set"] = set;
  const value_type m2 (_m2);

  {
    BOOST_CHECK (peek ("set", m2));
    BOOST_CHECK (*peek ("set", m2) == set);
    // BOOST_REQUIRE_EQUAL (*peek ("set", m2), set);
    // | Does not compile. Why?
  }
  {
    BOOST_CHECK (peek ("m1.l1", m2));
    BOOST_CHECK (*peek ("m1.l1", m2) == l1);
  }
}

BOOST_AUTO_TEST_CASE (peek_ref)
{
  using pnet::type::value::value_type;
  using pnet::type::value::poke;
  using pnet::type::value::peek;

  const value_type l = std::list<value_type>();

  value_type m;
  poke ("l", m, l);

  BOOST_CHECK (peek ("l", m));

  {
    const std::list<value_type>& g
      (boost::get<const std::list<value_type>&> (*peek ("l", m)));

    BOOST_CHECK (g.empty());
  }

  BOOST_CHECK (peek ("l", m));

  {
    std::list<value_type>& r
      (boost::get<std::list<value_type>&> (*peek ("l", m)));

    BOOST_CHECK (r.empty());

    r.push_back (19);
  }

  {
    const std::list<value_type>& g
      (boost::get<const std::list<value_type>&> (*peek ("l", m)));

    BOOST_CHECK (g.size() == 1);
    BOOST_CHECK (*g.begin() == value_type (19));
  }
}

BOOST_AUTO_TEST_CASE (poke)
{
  using pnet::type::value::value_type;
  using pnet::type::value::poke;
  using pnet::type::value::peek;

  value_type v;

  const value_type s (std::string ("s"));
  const value_type i (int (1));

  {
    poke ("s", v, s);

    BOOST_CHECK (peek ("s", v));
    BOOST_CHECK (*peek ("s", v) == s);
  }
  {
    poke ("i", v, i);
    BOOST_CHECK (peek ("i", v));
    BOOST_CHECK (*peek ("i", v) == i);

    poke ("i.i", v, i);
    BOOST_CHECK (peek ("i.i", v));
    BOOST_CHECK (*peek ("i.i", v) == i);

    BOOST_CHECK (peek ("i", v));
    BOOST_CHECK (peek ("i", *peek ("i", v)));
    BOOST_CHECK (*peek ("i", *peek ("i", v)) == i);
  }
}

BOOST_AUTO_TEST_CASE (signature_of_type)
{
  using pnet::type::value::of_type;
  using pnet::type::value::value_type;

#define CHECK(_name, _value)                                            \
  BOOST_CHECK (of_type (pnet::type::value::_name()) == value_type (_value))

  CHECK (CONTROL, we::type::literal::control());
  CHECK (BOOL, false);
  CHECK (INT, 0);
  CHECK (LONG, 0L);
  CHECK (UINT, 0U);
  CHECK (ULONG, 0UL);
  CHECK (FLOAT, 0.0f);
  CHECK (DOUBLE, 0.0);
  CHECK (CHAR, '\0');
  CHECK (STRING, std::string (""));
  CHECK (BITSET, bitsetofint::type());
  CHECK (BYTEARRAY, bytearray::type());
  CHECK (LIST, std::list<value_type>());
  CHECK (VECTOR, std::vector<value_type>());
  CHECK (SET, std::set<value_type>());
  std::map<value_type, value_type> m;
  CHECK (MAP, m);

#undef CHECK
}

BOOST_AUTO_TEST_CASE (signature_name_of)
{
  using pnet::type::value::name_of;
  using pnet::type::value::value_type;

#define CHECK(_name, _value)                                    \
  BOOST_CHECK (pnet::type::value::_name() == name_of (_value))

  CHECK (CONTROL, we::type::literal::control());
  CHECK (BOOL, false);
  CHECK (INT, 0);
  CHECK (LONG, 0L);
  CHECK (UINT, 0U);
  CHECK (ULONG, 0UL);
  CHECK (FLOAT, 0.0f);
  CHECK (DOUBLE, 0.0);
  CHECK (CHAR, '\0');
  CHECK (STRING, std::string (""));
  CHECK (BITSET, bitsetofint::type());
  CHECK (BYTEARRAY, bytearray::type());
  CHECK (LIST, std::list<value_type>());
  CHECK (VECTOR, std::vector<value_type>());
  CHECK (SET, std::set<value_type>());
  std::map<value_type, value_type> m;
  CHECK (MAP, m);

#undef CHECK
}

BOOST_AUTO_TEST_CASE (require_type)
{
  using pnet::type::value::value_type;
  using pnet::type::value::poke;
  using pnet::type::value::structured_type;
  using pnet::type::value::signature_type;
  using pnet::type::value::require_type;
  using pnet::type::value::exception::type_mismatch;
  using pnet::type::value::exception::missing_field;
  using pnet::type::value::exception::unknown_field;

#define OKAY(l,r) BOOST_CHECK_NO_THROW (require_type (l, r))

  OKAY (we::type::literal::control(), we::type::literal::control());
  OKAY (true, true);
  OKAY (true, false);
  OKAY (false, true);
  OKAY (false, false);
  OKAY (0, 1);
  OKAY (0L, 1L);
  OKAY (0U, 1U);
  OKAY (0UL, 1UL);
  OKAY (0.0f, 1.0f);
  OKAY (0.0, 1.0);
  OKAY ('a', 'b');
  OKAY (std::string ("foo"), std::string ("bar"));
  OKAY (bitsetofint::type().ins(0), bitsetofint::type().ins(1));
  OKAY (bytearray::type(), bytearray::type());

  BOOST_CHECK_THROW (require_type (0, 0U), type_mismatch);

  signature_type sig;
  value_type val;

  OKAY (sig, val);

  poke ("f", sig, 0);

  BOOST_CHECK_THROW (require_type (sig, val), type_mismatch);

  poke ("f", val, 1);

  OKAY (sig, val);

  poke ("g", sig, std::list<value_type>());

  BOOST_CHECK_THROW (require_type (sig, val), missing_field);

  poke ("g", val, std::list<value_type>());

  OKAY (sig, val);

  poke ("h", val, 'a');

  BOOST_CHECK_THROW (require_type (sig, val), unknown_field);

  poke ("h", sig, 'x');

  OKAY (sig, val);

  poke ("a.b", sig, 0);
  poke ("a.b", val, 1U);

  BOOST_CHECK_THROW (require_type (sig, val), type_mismatch);

#undef OKAY
}
