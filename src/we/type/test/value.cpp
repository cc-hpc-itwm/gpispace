// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_value
#include <boost/test/unit_test.hpp>

#include <we/type/value.hpp>
#include <we/type/value/name.hpp>
#include <we/type/value/name_of.hpp>
#include <we/type/value/of_type.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/wrap.hpp>
#include <we/type/value/unwrap.hpp>
#include <we/type/value/to_value.hpp>
#include <we/type/value/dump.hpp>
#include <we/field.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/num.hpp>
#include <fhg/util/indenter.hpp>

#include <fhg/util/boost/test/printer/list.hpp>
#include <fhg/util/boost/test/printer/set.hpp>
#include <fhg/util/boost/test/printer/map.hpp>

#include <fhg/util/xml.hpp>

#include <sstream>

using pnet::type::value::structured_type;

namespace
{
  template<typename T>
  void test_show_and_read_showed ( const T& x
                                 , const std::string& expected_show
                                 )
  {
    using pnet::type::value::value_type;
    using pnet::type::value::show;
    using pnet::type::value::read;
    using fhg::util::parse::position_string;

    {
      std::ostringstream oss;
      oss << show (value_type (x));
      BOOST_CHECK_EQUAL (expected_show, oss.str());
      const std::string inp (oss.str());
      position_string pos (inp);
      BOOST_CHECK_EQUAL (value_type (x), read (pos));
      BOOST_CHECK (pos.end());
    }
  }
}

BOOST_AUTO_TEST_CASE (num_type)
{
  using pnet::type::value::value_type;
  using fhg::util::num_type;

  BOOST_CHECK_EQUAL (value_type (23), value_type (num_type (23)));
  BOOST_CHECK_EQUAL (value_type (23U), value_type (num_type (23U)));
  BOOST_CHECK_EQUAL (value_type (23L), value_type (num_type (23L)));
  BOOST_CHECK_EQUAL (value_type (23UL), value_type (num_type (23UL)));
  BOOST_CHECK_EQUAL (value_type (23.0), value_type (num_type (23.0)));
  BOOST_CHECK_EQUAL (value_type (23.0f), value_type (num_type (23.0f)));
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
    test_show_and_read_showed (l, "List ()");
    l.push_back (value_type (int (3)));
    test_show_and_read_showed (l, "List (3)");
    l.push_back (value_type (long (3)));
    test_show_and_read_showed (l, "List (3, 3L)");
  }
  {
    std::set<value_type> s;
    test_show_and_read_showed (s, "Set {}");
    s.insert (std::string ("foo"));
    s.insert (3.141);
    s.insert (3.141);
    s.insert (3.141f);
    test_show_and_read_showed (s, "Set {3.14100f, 3.14100, \"foo\"}");
  }
  {
    std::map<value_type, value_type> m;
    test_show_and_read_showed (m, "Map []");
    m[std::string ("foo")] = 314U;
    m[14] = 14;
    m[std::map<value_type,value_type>()] = 14;
    test_show_and_read_showed
      (m, "Map [14 -> 14, \"foo\" -> 314U, Map [] -> 14]");
  }
  {
    structured_type m;
    test_show_and_read_showed (m, "Struct []");
    m.push_back (std::make_pair ("foo", 314U));
    m.push_back (std::make_pair ("bar", 14UL));
    m.push_back (std::make_pair ("baz", std::list<value_type>()));
    test_show_and_read_showed
      (m, "Struct [foo := 314U, bar := 14UL, baz := List ()]");
  }
}

BOOST_AUTO_TEST_CASE (_read)
{
  using pnet::type::value::value_type;
  using pnet::type::value::read;

  BOOST_CHECK_EQUAL (value_type (true), read ("true"));
  BOOST_CHECK_EQUAL (value_type (false), read ("false"));
  BOOST_CHECK_EQUAL (value_type (we::type::literal::control()), read ("[]"));
  BOOST_CHECK_EQUAL (value_type ('a'), read ("'a'"));
  BOOST_CHECK_EQUAL (value_type (std::string ("foo")), read ("\"foo\""));
  BOOST_CHECK_EQUAL (value_type (std::string ("\"")), read ("\"\\\"\""));
  BOOST_CHECK_EQUAL (value_type (std::string ("\\\"")), read ("\"\\\\\\\"\""));
  BOOST_CHECK_EQUAL (value_type (std::string ("\"foo\"")), read ("\"\\\"foo\\\"\""));

  BOOST_CHECK_THROW (read ("\"\\n\""), fhg::util::parse::error::expected);

  BOOST_CHECK_EQUAL (value_type (23), read ("23"));
  BOOST_CHECK_EQUAL (value_type (23U), read ("23U"));
  BOOST_CHECK_EQUAL (value_type (23L), read ("23L"));
  BOOST_CHECK_EQUAL (value_type (23UL), read ("23UL"));
  BOOST_CHECK_EQUAL (value_type (2.3f), read ("2.3f"));
  BOOST_CHECK_EQUAL (value_type (2.3), read ("2.3"));

  {
    bitsetofint::type bs;
    BOOST_CHECK_EQUAL (value_type (bs), read ("{}"));
    bs.ins (0);
    BOOST_CHECK_EQUAL (value_type (bs), read ("{ 1}"));
    bs.ins (64);
    BOOST_CHECK_EQUAL (value_type (bs), read ("{ 1 1}"));
  }
  {
    bytearray::type ba;
    BOOST_CHECK_EQUAL (value_type (ba), read ("y()"));
    ba.push_back (' ');
    BOOST_CHECK_EQUAL (value_type (ba), read ("y( 32)"));
    ba.push_back ('A');
    BOOST_CHECK_EQUAL (value_type (ba), read ("y( 32 65)"));
  }
  {
    std::map<value_type, value_type> m;

    BOOST_CHECK_EQUAL (value_type (m), read ("Map []"));

    m[value_type ('a')] = value_type (std::string ("foo"));
    m[value_type (std::string ("foo"))] = value_type ('a');

    BOOST_CHECK_EQUAL (value_type (m), read ("Map ['a'->\"foo\",\"foo\"->'a']"));
  }

  {
    structured_type m;

    BOOST_CHECK_EQUAL (value_type (m), read ("Struct []"));

    m.push_back (std::make_pair ("a", value_type ('a')));
    m.push_back (std::make_pair ("foo", value_type ('b')));

    BOOST_CHECK_EQUAL (value_type (m), read ("Struct [a:='a',foo:='b']"));
  }

  {
    std::list<value_type> l;
    std::map<value_type, value_type> m;
    std::string input ("List( ) Map[]");
    fhg::util::parse::position_string pos (input);

    BOOST_CHECK_EQUAL (value_type (l), read (pos));
    BOOST_CHECK_EQUAL (value_type (m), read (pos));
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
  boost::get<structured_type&> (m1).push_back (std::make_pair ("i", i));
  boost::get<structured_type&> (m1).push_back (std::make_pair ("s", s));
  boost::get<structured_type&> (m1).push_back (std::make_pair ("l1", l1));

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
  _m2.push_back (std::make_pair ("m1", m1));
  _m2.push_back (std::make_pair ("l2", l2));
  _m2.push_back (std::make_pair ("mv", mv));
  _m2.push_back (std::make_pair ("set", set));
  const value_type m2 (_m2);

  {
    BOOST_CHECK_NE (peek ("set", m2), boost::none);
    BOOST_CHECK_EQUAL (*peek ("set", m2), set);
    // BOOST_REQUIRE_EQUAL (*peek ("set", m2), set);
    // | Does not compile. Why?
  }
  {
    BOOST_CHECK_NE (peek ("m1.l1", m2), boost::none);
    BOOST_CHECK_EQUAL (*peek ("m1.l1", m2), l1);
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

  BOOST_CHECK_NE (peek ("l", m), boost::none);

  {
    const std::list<value_type>& g
      (boost::get<const std::list<value_type>&> (*peek ("l", m)));

    BOOST_CHECK (g.empty());
  }

  BOOST_CHECK_NE (peek ("l", m), boost::none);

  {
    std::list<value_type>& r
      (boost::get<std::list<value_type>&> (*peek ("l", m)));

    BOOST_CHECK (r.empty());

    r.push_back (19);
  }

  {
    const std::list<value_type>& g
      (boost::get<const std::list<value_type>&> (*peek ("l", m)));

    BOOST_CHECK_EQUAL (g.size(), 1);
    BOOST_CHECK_EQUAL (*g.begin(), value_type (19));
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

    BOOST_CHECK_NE (peek ("s", v), boost::none);
    BOOST_CHECK_EQUAL (*peek ("s", v), s);
  }
  {
    poke ("i", v, i);
    BOOST_CHECK_NE (peek ("i", v), boost::none);
    BOOST_CHECK_EQUAL (*peek ("i", v), i);

    poke ("i.i", v, i);
    BOOST_CHECK_NE (peek ("i.i", v), boost::none);
    BOOST_CHECK_EQUAL (*peek ("i.i", v), i);

    BOOST_CHECK_NE (peek ("i", v), boost::none);
    BOOST_CHECK_NE (peek ("i", *peek ("i", v)), boost::none);
    BOOST_CHECK_EQUAL (*peek ("i", *peek ("i", v)), i);
  }
}

BOOST_AUTO_TEST_CASE (signature_of_type)
{
  using pnet::type::value::of_type;
  using pnet::type::value::value_type;

#define CHECK(_name, _value)                                            \
  BOOST_CHECK_EQUAL (of_type (pnet::type::value::_name()), value_type (_value))

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
  BOOST_CHECK_EQUAL (pnet::type::value::_name(), name_of (_value))

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
  CHECK (SET, std::set<value_type>());
  std::map<value_type, value_type> m;
  CHECK (MAP, m);

#undef CHECK
}

BOOST_AUTO_TEST_CASE (unwrap)
{
  namespace value = pnet::type::value;

  //! \todo implement me
  {
    std::list<value::value_type> lv;
    std::list<long> lt;

    BOOST_CHECK_EQUAL (lt, value::unwrap<long> (lv));
  }

  {
    std::list<value::value_type> lv;
    lv.push_back (0L);
    lv.push_back (1L);

    std::list<long> lt;
    lt.push_back (0L);
    lt.push_back (1L);

    BOOST_CHECK_EQUAL (lt, value::unwrap<long> (lv));
  }
}

BOOST_AUTO_TEST_CASE (wrap)
{
  namespace value = pnet::type::value;

  typedef std::map<value::value_type, value::value_type> map_vv_type;

#define CHECK_EMPTY_LIST(_t...)                        \
  BOOST_CHECK_EQUAL ( std::list<value::value_type>()   \
                    , value::wrap (std::list<_t>())    \
                    )

  CHECK_EMPTY_LIST (we::type::literal::control);
  CHECK_EMPTY_LIST (bool);
  CHECK_EMPTY_LIST (int);
  CHECK_EMPTY_LIST (long);
  CHECK_EMPTY_LIST (unsigned int);
  CHECK_EMPTY_LIST (unsigned long);
  CHECK_EMPTY_LIST (float);
  CHECK_EMPTY_LIST (double);
  CHECK_EMPTY_LIST (char);
  CHECK_EMPTY_LIST (std::string);
  CHECK_EMPTY_LIST (bitsetofint::type);
  CHECK_EMPTY_LIST (bytearray::type);
  CHECK_EMPTY_LIST (std::list<value::value_type>);
  CHECK_EMPTY_LIST (std::set<value::value_type>);
  CHECK_EMPTY_LIST (map_vv_type);
  CHECK_EMPTY_LIST (value::value_type);

#undef CHECK_EMPTY_LIST

#define CHECK_EMPTY_SET(_t...)                       \
  BOOST_CHECK_EQUAL ( std::set<value::value_type>()  \
                    , value::wrap (std::set<_t>())   \
                    )

  CHECK_EMPTY_SET (we::type::literal::control);
  CHECK_EMPTY_SET (bool);
  CHECK_EMPTY_SET (int);
  CHECK_EMPTY_SET (long);
  CHECK_EMPTY_SET (unsigned int);
  CHECK_EMPTY_SET (unsigned long);
  CHECK_EMPTY_SET (float);
  CHECK_EMPTY_SET (double);
  CHECK_EMPTY_SET (char);
  CHECK_EMPTY_SET (std::string);
  CHECK_EMPTY_SET (bitsetofint::type);
  CHECK_EMPTY_SET (bytearray::type);
  CHECK_EMPTY_SET (std::list<value::value_type>);
  CHECK_EMPTY_SET (std::set<value::value_type>);
  CHECK_EMPTY_SET (map_vv_type);
  CHECK_EMPTY_SET (value::value_type);

#undef CHECK_EMPTY_SET

  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< we::type::literal::control,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bitsetofint::type,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bytearray::type,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,bytearray::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,value::value_type >()));

  {
    std::list<long> ll;
    ll.push_back (0L);
    ll.push_back (9L);
    ll.push_back (3L);

    std::list<value::value_type> lv;
    lv.push_back (0L);
    lv.push_back (9L);
    lv.push_back (3L);

    BOOST_CHECK_EQUAL (lv, value::wrap (ll));
  }

  {
    std::set<double> sd;
    sd.insert (0.0);

    std::set<value::value_type> sv;
    sv.insert (0.0);

    BOOST_CHECK_EQUAL (sv, value::wrap (sd));
  }
}

namespace
{
  struct s_type
  {
    s_type (int x, int y)
      : _x (x)
      , _y (y)
    {}
    int x() const
    {
      return _x;
    }
    int y() const
    {
      return _y;
    }
  private:
    int _x;
    int _y;
  };

  struct by_x
  {
    bool operator() (s_type const& a, s_type const& b) const
    {
      return a.x() < b.x();
    }
  };

  struct by_y
  {
    bool operator() (s_type const& a, s_type const& b) const
    {
      return a.y() < b.y();
    }
  };

  bool operator< (s_type const& a, s_type const& b)
  {
    return by_x() (a, b);
  }
}

namespace pnet
{
  namespace type
  {
    namespace value
    {
      template<>
        inline value_type to_value<s_type> (s_type const& x)
      {
        value_type v;
        poke ("x", v, x.x());
        poke ("y", v, x.y());
        return v;
      }
    }
  }
}

BOOST_AUTO_TEST_CASE (wrap_generated)
{
  s_type const s (1, 2);

  {
    std::list<s_type> const ls (10, s);

    std::list<pnet::type::value::value_type> const lv
      (pnet::type::value::wrap (ls));

    BOOST_CHECK_EQUAL (ls.size(), lv.size());

    BOOST_FOREACH (const pnet::type::value::value_type& v, lv)
    {
      BOOST_CHECK_EQUAL (pnet::type::value::to_value (s), v);
    }
  }

  {
    std::set<s_type> ss;
    ss.insert (s);
    ss.insert (s);
    ss.insert (s);

    std::set<pnet::type::value::value_type> const sv
      (pnet::type::value::wrap (ss));

    BOOST_CHECK_EQUAL (ss.size(), sv.size());

    BOOST_FOREACH (const pnet::type::value::value_type& v, sv)
    {
      BOOST_CHECK_EQUAL (pnet::type::value::to_value (s), v);
    }
  }

  {
    std::map<s_type, s_type> ms;
    ms.insert (std::make_pair (s, s));

    std::map< pnet::type::value::value_type
            , pnet::type::value::value_type
            > const mv
      (pnet::type::value::wrap (ms));

    BOOST_CHECK_EQUAL (ms.size(), mv.size());

    BOOST_CHECK_EQUAL (pnet::type::value::to_value (s), mv.begin()->first);
    BOOST_CHECK_EQUAL (pnet::type::value::to_value (s), mv.begin()->second);
  }
}

namespace
{
  void dump_okay (std::string const& v, std::string const& expected)
  {
    std::ostringstream oss;
    fhg::util::xml::xmlstream os (oss);

    pnet::type::value::dump (os, pnet::type::value::read (v));

    BOOST_REQUIRE_EQUAL (oss.str(), expected);
  }
}

BOOST_AUTO_TEST_CASE (dump)
{
  dump_okay ( "Struct []"
            , ""
            );
  dump_okay ( "Struct [foo := 0L]"
            , "<property key=\"foo\">0L</property>"
            );
  dump_okay ( "Struct [foo := 0L, bar := List (1U, 2)]"
            , "<property key=\"foo\">0L</property>"
              "<property key=\"bar\">List (1U, 2)</property>"
            );
  dump_okay ( "Struct [fhg := Struct [ drts := Struct"
              "                             [ schedule := Struct"
              "                                        [ num_worker := 12L"
              "                                        , memory := 4294967296L"
              "                                        ]"
              "                             ]"
              "                      , pnete := Struct"
              "                              [ invisible := true"
              "                              , position := Struct [ x := 24"
              "                                                   , y := 36"
              "                                                   ]"
              "                              ]"
              "                      ]"
              "       ]"
            , "<properties name=\"fhg\">\n"
              "  <properties name=\"drts\">\n"
              "    <properties name=\"schedule\">\n"
              "      <property key=\"num_worker\">12L</property>\n"
              "      <property key=\"memory\">4294967296L</property>\n"
              "    </properties>\n"
              "  </properties>\n"
              "  <properties name=\"pnete\">\n"
              "    <property key=\"invisible\">true</property>\n"
              "    <properties name=\"position\">\n"
              "      <property key=\"x\">24</property>\n"
              "      <property key=\"y\">36</property>\n"
              "    </properties>\n"
              "  </properties>\n"
              "</properties>"
            );
}
