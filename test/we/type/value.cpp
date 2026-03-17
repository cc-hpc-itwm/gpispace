// Copyright (C) 2013-2016,2018,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/we/field.hpp>
#include <gspc/we/type/shared.hpp>
#include <gspc/we/type/value.hpp>
#include <gspc/we/type/value/dump.hpp>
#include <gspc/we/type/value/name.hpp>
#include <gspc/we/type/value/name_of.hpp>
#include <gspc/we/type/value/peek.hpp>
#include <gspc/we/type/value/poke.hpp>
#include <gspc/we/type/value/positions.hpp>
#include <gspc/we/type/value/read.hpp>
#include <gspc/we/type/value/serialize.hpp>
#include <gspc/we/type/value/show.hpp>
#include <gspc/we/type/value/to_value.hpp>
#include <gspc/we/type/value/unwrap.hpp>
#include <gspc/we/type/value/wrap.hpp>

#include <gspc/testing/printer/we/type/value.hpp>
#include <gspc/we/type/value/path/split.hpp>

#include <gspc/util/indenter.hpp>
#include <gspc/util/num.hpp>
#include <gspc/util/parse/error.hpp>
#include <gspc/util/parse/position.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/list.hpp>
#include <gspc/testing/printer/map.hpp>
#include <gspc/testing/printer/optional.hpp>
#include <gspc/testing/printer/set.hpp>
#include <gspc/testing/require_exception.hpp>

#include <gspc/util/xml.hpp>

#include <gspc/we/type/value/show.formatter.hpp>
#include <fmt/core.h>
#include <sstream>

using gspc::pnet::type::value::structured_type;

namespace
{
  template<typename T>
  void test_show_and_read_showed ( T const& x
                                 , std::string const& expected_show
                                 )
  {
    using gspc::pnet::type::value::value_type;

    {
      std::ostringstream oss;
      oss << gspc::pnet::type::value::show (value_type (x));
      BOOST_CHECK_EQUAL (expected_show, oss.str());
      const std::string inp (oss.str());
      gspc::util::parse::position pos (inp);
      BOOST_CHECK_EQUAL (value_type (x), gspc::pnet::type::value::read (pos));
      BOOST_CHECK (pos.end());
    }

    BOOST_CHECK_EQUAL
      ( gspc::pnet::type::value::from_string
          (gspc::pnet::type::value::to_string (value_type (x)))
      , value_type (x)
      );
  }
}

BOOST_AUTO_TEST_CASE (num_type)
{
  using gspc::pnet::type::value::value_type;
  using gspc::util::num_type;

  auto mk_value
    { [] (auto const& n)
      {
        return std::visit
          ( [] (auto const& v)
            {
              return value_type {v};
            }
          , n
          );
      }
    };

  BOOST_CHECK_EQUAL (value_type (23), mk_value (num_type (23)));
  BOOST_CHECK_EQUAL (value_type (23U), mk_value (num_type (23U)));
  BOOST_CHECK_EQUAL (value_type (23L), mk_value (num_type (23L)));
  BOOST_CHECK_EQUAL (value_type (23UL), mk_value (num_type (23UL)));
  BOOST_CHECK_EQUAL (value_type (gspc::pnet::type::value::bigint_type (23)), mk_value (num_type (gspc::pnet::type::value::bigint_type (23))));
  BOOST_CHECK_EQUAL (value_type (23.0), mk_value (num_type (23.0)));
  BOOST_CHECK_EQUAL (value_type (23.0f), mk_value (num_type (23.0f)));
}

BOOST_AUTO_TEST_CASE (show_and_read_showed)
{
  using gspc::pnet::type::value::value_type;

  test_show_and_read_showed (gspc::we::type::literal::control(), "[]");
  test_show_and_read_showed (true, "true");
  test_show_and_read_showed (false, "false");
  test_show_and_read_showed (0, "0");
  test_show_and_read_showed (23, "23");
  test_show_and_read_showed (42L, "42L");
  test_show_and_read_showed (-3L, "-3L");
  test_show_and_read_showed (2718U, "2718U");
  test_show_and_read_showed (3141UL, "3141UL");
  test_show_and_read_showed (gspc::pnet::type::value::bigint_type (0), "0A");
  test_show_and_read_showed (gspc::pnet::type::value::bigint_type (42), "42A");
  test_show_and_read_showed (gspc::pnet::type::value::bigint_type (-3), "-3A");
  test_show_and_read_showed (3.14159f, "3.14159f");
  test_show_and_read_showed (3e30f, "3.00000e+30f");
  test_show_and_read_showed (3.14159, "3.14159");
  test_show_and_read_showed (3e30, "3.00000e+30");
  test_show_and_read_showed (0.0, "0.00000");
  test_show_and_read_showed ('\0', std::string("'\0'", 3));
  test_show_and_read_showed ('a', "'a'");
  test_show_and_read_showed (std::string(), "\"\"");
  test_show_and_read_showed (std::string("foo"), "\"foo\"");
  test_show_and_read_showed (gspc::pnet::type::bitsetofint::type(), "{}");
  test_show_and_read_showed (gspc::pnet::type::bitsetofint::type().ins (0), "{ 1}");
  test_show_and_read_showed (gspc::pnet::type::bitsetofint::type().ins (0).ins (64), "{ 1 1}");
  test_show_and_read_showed (gspc::we::type::bytearray(), "y()");
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
  // shared types - round-trip show and read
  {
    test_show_and_read_showed
      ( gspc::we::type::shared {std::string ("test"), "cleanup"}
      , "shared_cleanup (\"test\")"
      );
    test_show_and_read_showed
      ( gspc::we::type::shared {42, "data"}
      , "shared_data (42)"
      );
    test_show_and_read_showed
      ( gspc::we::type::shared {std::list<value_type> {1, 2L, 3U}, "result"}
      , "shared_result (List (1, 2L, 3U))"
      );
  }
}

BOOST_AUTO_TEST_CASE (_read)
{
  using gspc::pnet::type::value::value_type;
  using gspc::pnet::type::value::read;

  BOOST_CHECK_EQUAL (value_type (true), read ("true"));
  BOOST_CHECK_EQUAL (value_type (false), read ("false"));
  BOOST_CHECK_EQUAL (value_type (gspc::we::type::literal::control()), read ("[]"));
  BOOST_CHECK_EQUAL (value_type ('a'), read ("'a'"));
  BOOST_CHECK_EQUAL (value_type (std::string ("foo")), read ("\"foo\""));
  BOOST_CHECK_EQUAL (value_type (std::string ("\"")), read ("\"\\\"\""));
  BOOST_CHECK_EQUAL (value_type (std::string ("\\\"")), read ("\"\\\\\\\"\""));
  BOOST_CHECK_EQUAL (value_type (std::string ("\"foo\"")), read ("\"\\\"foo\\\"\""));

  BOOST_CHECK_THROW (read ("\"\\n\""), gspc::util::parse::error::expected);

  BOOST_CHECK_EQUAL (value_type (23), read ("23"));
  BOOST_CHECK_EQUAL (value_type (23U), read ("23U"));
  BOOST_CHECK_EQUAL (value_type (23L), read ("23L"));
  BOOST_CHECK_EQUAL (value_type (23UL), read ("23UL"));
  BOOST_CHECK_EQUAL (value_type (2.3f), read ("2.3f"));
  BOOST_CHECK_EQUAL (value_type (2.3), read ("2.3"));

  {
    gspc::pnet::type::bitsetofint::type bs;
    BOOST_CHECK_EQUAL (value_type (bs), read ("{}"));
    bs.ins (0);
    BOOST_CHECK_EQUAL (value_type (bs), read ("{ 1}"));
    bs.ins (64);
    BOOST_CHECK_EQUAL (value_type (bs), read ("{ 1 1}"));
  }
  {
    gspc::we::type::bytearray ba;
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
    gspc::util::parse::position pos (input);

    BOOST_CHECK_EQUAL (value_type (l), read (pos));
    BOOST_CHECK_EQUAL (value_type (m), read (pos));
    BOOST_CHECK (pos.end());
 }

  // shared value read tests
  {
    // shared with string value
    BOOST_CHECK_EQUAL
      ( value_type (gspc::we::type::shared {std::string ("foo"), "cleanup"})
      , read ("shared_cleanup (\"foo\")")
      );

    // shared with int value
    BOOST_CHECK_EQUAL
      ( value_type (gspc::we::type::shared {42, "cleanup"})
      , read ("shared_cleanup (42)")
      );

    // shared with long value
    BOOST_CHECK_EQUAL
      ( value_type (gspc::we::type::shared {42L, "result"})
      , read ("shared_result (42L)")
      );

    // shared with list value
    std::list<value_type> list_value;
    list_value.push_back (1);
    list_value.push_back (2L);
    BOOST_CHECK_EQUAL
      ( value_type (gspc::we::type::shared {list_value, "data"})
      , read ("shared_data (List (1, 2L))")
      );

    // shared with struct value
    structured_type struct_value;
    struct_value.push_back (std::make_pair ("x", 10));
    struct_value.push_back (std::make_pair ("y", 20));
    BOOST_CHECK_EQUAL
      ( value_type (gspc::we::type::shared {struct_value, "point"})
      , read ("shared_point (Struct [x := 10, y := 20])")
      );

    // shared with nested shared value
    gspc::we::type::shared inner {std::string ("inner"), "inner_cleanup"};
    BOOST_CHECK_EQUAL
      ( value_type (gspc::we::type::shared {value_type {inner}, "outer_cleanup"})
      , read ("shared_outer_cleanup (shared_inner_cleanup (\"inner\"))")
      );

    // shared with missing cleanup place name throws parse error
    {
      std::string const input {"shared_ (42)"};
      gspc::util::parse::position pos {input};
      // Advance position to where the error occurs (after "shared_")
      for (auto i {0}; i < 7; ++i) { ++pos; }

      gspc::testing::require_exception
        ( [&input]
          {
            std::ignore = read (input);
          }
        , gspc::util::parse::error::expected ("cleanup place name", pos)
        );
    }
  }
}

BOOST_AUTO_TEST_CASE (peek)
{
  using gspc::pnet::type::value::value_type;
  using gspc::pnet::type::value::peek;

  const value_type s (std::string ("s"));

  BOOST_CHECK_EQUAL (::boost::get<std::string> (s), "s");
  static_assert ( std::is_same < std::string const&
                               , decltype (::boost::get<std::string> (s))
                               >::value
                , "getting from const value_type returns const&"
                );

  value_type i (int (0));

  BOOST_CHECK_EQUAL (::boost::get<int> (i), 0);

  ::boost::get<int> (i) = 1;

  BOOST_CHECK_EQUAL (::boost::get<int> (i), 1);

  value_type l1;
  {
    std::list<value_type> _l1;
    _l1.push_back (i);
    _l1.push_back (s);
    _l1.push_back (3L);
    l1 = _l1;
  }

  value_type m1 = structured_type();
  ::boost::get<structured_type> (m1).push_back (std::make_pair ("i", i));
  ::boost::get<structured_type> (m1).push_back (std::make_pair ("s", s));
  ::boost::get<structured_type> (m1).push_back (std::make_pair ("l1", l1));

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
    BOOST_CHECK_NE (peek ("set", m2), std::nullopt);
    BOOST_CHECK_EQUAL (peek ("set", m2)->get(), set);
  }
  {
    BOOST_CHECK_NE (peek ("m1.l1", m2), std::nullopt);
    BOOST_CHECK_EQUAL (peek ("m1.l1", m2)->get(), l1);
  }
}

BOOST_AUTO_TEST_CASE (peek_ref)
{
  using gspc::pnet::type::value::value_type;
  using gspc::pnet::type::value::poke;
  using gspc::pnet::type::value::peek;

  const value_type l = std::list<value_type>();

  value_type m;
  poke ("l", m, l);

  BOOST_CHECK_NE (peek ("l", m), std::nullopt);

  {
    std::list<value_type> const& g
      (::boost::get<std::list<value_type>> (peek ("l", m)->get()));

    BOOST_CHECK (g.empty());
  }

  BOOST_CHECK_NE (peek ("l", m), std::nullopt);

  {
    auto& r
      (::boost::get<std::list<value_type>> (peek ("l", m)->get()));

    BOOST_CHECK (r.empty());

    r.push_back (19);
  }

  {
    std::list<value_type> const& g
      (::boost::get<std::list<value_type>> (peek ("l", m)->get()));

    BOOST_CHECK_EQUAL (g.size(), 1U);
    BOOST_CHECK_EQUAL (*g.begin(), value_type (19));
  }
}

BOOST_AUTO_TEST_CASE (poke)
{
  using gspc::pnet::type::value::value_type;
  using gspc::pnet::type::value::poke;
  using gspc::pnet::type::value::peek;

  value_type v;

  const value_type s (std::string ("s"));
  const value_type i (int (1));

  {
    poke ("s", v, s);

    BOOST_CHECK_NE (peek ("s", v), std::nullopt);
    BOOST_CHECK_EQUAL (peek ("s", v)->get(), s);
  }
  {
    poke ("i", v, i);
    BOOST_CHECK_NE (peek ("i", v), std::nullopt);
    BOOST_CHECK_EQUAL (peek ("i", v)->get(), i);

    poke ("i.i", v, i);
    BOOST_CHECK_NE (peek ("i.i", v), std::nullopt);
    BOOST_CHECK_EQUAL (peek ("i.i", v)->get(), i);

    BOOST_CHECK_NE (peek ("i", v), std::nullopt);
    BOOST_CHECK_NE (peek ("i", peek ("i", v)->get()), std::nullopt);
    BOOST_CHECK_EQUAL (peek ("i", peek ("i", v)->get())->get(), i);
  }
}

BOOST_AUTO_TEST_CASE (signature_name_of)
{
  using gspc::pnet::type::value::name_of;
  using gspc::pnet::type::value::value_type;

#define CHECK(_name, _value)                                    \
  BOOST_CHECK_EQUAL (gspc::pnet::type::value::_name(), name_of (_value))

  CHECK (CONTROL, gspc::we::type::literal::control());
  CHECK (BOOL, false);
  CHECK (INT, 0);
  CHECK (LONG, 0L);
  CHECK (UINT, 0U);
  CHECK (ULONG, 0UL);
  CHECK (FLOAT, 0.0f);
  CHECK (DOUBLE, 0.0);
  CHECK (CHAR, '\0');
  CHECK (STRING, std::string (""));
  CHECK (BITSET, gspc::pnet::type::bitsetofint::type());
  CHECK (BYTEARRAY, gspc::we::type::bytearray());
  CHECK (LIST, std::list<value_type>());
  CHECK (SET, std::set<value_type>());
  std::map<value_type, value_type> m;
  CHECK (MAP, m);

#undef CHECK
}

namespace
{
  struct user_defined_type {};

  bool operator== (user_defined_type const&, user_defined_type const&)
  {
    return true;
  }
}

GSPC_BOOST_TEST_LOG_VALUE_PRINTER (user_defined_type, os, /**/)
{
  os << "user_defined_type()";
}



    namespace gspc::pnet::type::value
    {
      template<>
        inline value_type to_value<user_defined_type> (user_defined_type const&)
      {
        return value_type();
      }
      template<>
        inline user_defined_type from_value<user_defined_type>
          (value_type const&)
      {
        return user_defined_type();
      }
    }



BOOST_AUTO_TEST_CASE (unwrap)
{
  namespace value = gspc::pnet::type::value;

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

  {
    std::list<value::value_type> lv;
    lv.push_back (gspc::pnet::type::value::to_value (user_defined_type()));
    lv.push_back (gspc::pnet::type::value::to_value (user_defined_type()));

    std::list<user_defined_type> lt;
    lt.push_back (user_defined_type());
    lt.push_back (user_defined_type());

    BOOST_CHECK_EQUAL (lt, value::unwrap<user_defined_type> (lv));
  }
}

BOOST_AUTO_TEST_CASE (wrap)
{
  namespace value = gspc::pnet::type::value;

  using map_vv_type = std::map<value::value_type, value::value_type>;

#define CHECK_EMPTY_LIST(_t...)                        \
  BOOST_CHECK_EQUAL ( std::list<value::value_type>()   \
                    , value::wrap (std::list<_t>())    \
                    )

  CHECK_EMPTY_LIST (gspc::we::type::literal::control);
  CHECK_EMPTY_LIST (bool);
  CHECK_EMPTY_LIST (int);
  CHECK_EMPTY_LIST (long);
  CHECK_EMPTY_LIST (unsigned int);
  CHECK_EMPTY_LIST (unsigned long);
  CHECK_EMPTY_LIST (float);
  CHECK_EMPTY_LIST (double);
  CHECK_EMPTY_LIST (char);
  CHECK_EMPTY_LIST (std::string);
  CHECK_EMPTY_LIST (gspc::pnet::type::bitsetofint::type);
  CHECK_EMPTY_LIST (gspc::we::type::bytearray);
  CHECK_EMPTY_LIST (std::list<value::value_type>);
  CHECK_EMPTY_LIST (std::set<value::value_type>);
  CHECK_EMPTY_LIST (map_vv_type);
  CHECK_EMPTY_LIST (value::value_type);

#undef CHECK_EMPTY_LIST

#define CHECK_EMPTY_SET(_t...)                       \
  BOOST_CHECK_EQUAL ( std::set<value::value_type>()  \
                    , value::wrap (std::set<_t>())   \
                    )

  CHECK_EMPTY_SET (gspc::we::type::literal::control);
  CHECK_EMPTY_SET (bool);
  CHECK_EMPTY_SET (int);
  CHECK_EMPTY_SET (long);
  CHECK_EMPTY_SET (unsigned int);
  CHECK_EMPTY_SET (unsigned long);
  CHECK_EMPTY_SET (float);
  CHECK_EMPTY_SET (double);
  CHECK_EMPTY_SET (char);
  CHECK_EMPTY_SET (std::string);
  CHECK_EMPTY_SET (gspc::pnet::type::bitsetofint::type);
  CHECK_EMPTY_SET (gspc::we::type::bytearray);
  CHECK_EMPTY_SET (std::list<value::value_type>);
  CHECK_EMPTY_SET (std::set<value::value_type>);
  CHECK_EMPTY_SET (map_vv_type);
  CHECK_EMPTY_SET (value::value_type);

#undef CHECK_EMPTY_SET

  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::literal::control,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< bool,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< int,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< long,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned int,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< unsigned long,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< float,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< double,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< char,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::string,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::pnet::type::bitsetofint::type,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< gspc::we::type::bytearray,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::list<value::value_type>,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::set<value::value_type>,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,gspc::we::type::bytearray >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,std::list<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,std::set<value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,std::map<value::value_type,value::value_type> >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< std::map<value::value_type,value::value_type>,value::value_type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,gspc::we::type::literal::control >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,bool >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,unsigned int >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,unsigned long >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,float >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,double >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,char >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,std::string >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,gspc::pnet::type::bitsetofint::type >()));
  BOOST_CHECK_EQUAL (map_vv_type(), value::wrap (std::map< value::value_type,gspc::we::type::bytearray >()));
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

  bool operator< (s_type const& a, s_type const& b)
  {
    return a.x() < b.x();
  }
}



    namespace gspc::pnet::type::value
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



BOOST_AUTO_TEST_CASE (wrap_generated)
{
  s_type const s (1, 2);

  {
    std::list<s_type> const ls (10, s);

    std::list<gspc::pnet::type::value::value_type> const lv
      (gspc::pnet::type::value::wrap (ls));

    BOOST_CHECK_EQUAL (ls.size(), lv.size());

    for (gspc::pnet::type::value::value_type const& v : lv)
    {
      BOOST_CHECK_EQUAL (gspc::pnet::type::value::to_value (s), v);
    }
  }

  {
    std::set<s_type> ss;
    ss.insert (s);
    ss.insert (s);
    ss.insert (s);

    std::set<gspc::pnet::type::value::value_type> const sv
      (gspc::pnet::type::value::wrap (ss));

    BOOST_CHECK_EQUAL (ss.size(), sv.size());

    for (gspc::pnet::type::value::value_type const& v : sv)
    {
      BOOST_CHECK_EQUAL (gspc::pnet::type::value::to_value (s), v);
    }
  }

  {
    std::map<s_type, s_type> ms;
    ms.insert (std::make_pair (s, s));

    std::map< gspc::pnet::type::value::value_type
            , gspc::pnet::type::value::value_type
            > const mv
      (gspc::pnet::type::value::wrap (ms));

    BOOST_CHECK_EQUAL (ms.size(), mv.size());

    BOOST_CHECK_EQUAL (gspc::pnet::type::value::to_value (s), mv.begin()->first);
    BOOST_CHECK_EQUAL (gspc::pnet::type::value::to_value (s), mv.begin()->second);
  }
}

namespace
{
  void dump_okay (std::string const& v, std::string const& expected)
  {
    std::ostringstream oss;
    gspc::util::xml::xmlstream os (oss);

    gspc::pnet::type::value::dump (os, gspc::pnet::type::value::read (v));

    BOOST_REQUIRE_EQUAL (oss.str(), expected);
  }

  void dump_throw_plain (std::string const& v)
  {
    std::ostringstream oss;
    gspc::util::xml::xmlstream os (oss);

    gspc::pnet::type::value::value_type const value (gspc::pnet::type::value::read (v));

    gspc::testing::require_exception
      ( [&os, &value] { gspc::pnet::type::value::dump (os, value); }
      , std::runtime_error
          { fmt::format ( "cannot dump the plain value '{}'"
                        , gspc::pnet::type::value::show (value)
                        )
          }
      );
  }

  void dump_throw_depth ( std::string const& v
                        , std::string const& key
                        , std::string const& val
                        )
  {
    std::ostringstream oss;
    gspc::util::xml::xmlstream os (oss);

    gspc::testing::require_exception
      ( [&os, &v] { gspc::pnet::type::value::dump (os, gspc::pnet::type::value::read (v)); }
      , std::runtime_error
          { fmt::format ( "cannot dump the single level property"
                          " with key '{}' and value '{}'"
                        , key
                        , val
                        )
          }
      );
  }
}

BOOST_AUTO_TEST_CASE (dump)
{
  dump_okay ( "Struct[]", "");
  dump_okay ( "Struct [root := Struct[]]"
            , "<properties name=\"root\"/>"
            );
  dump_okay ( "Struct [root := Struct [k := []]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">[]</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := true]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">true</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := 0]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">0</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := 0U]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">0U</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := 0L]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">0L</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := 0UL]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">0UL</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := 0.0f]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">0.00000f</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := 0.0]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">0.00000</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := 'c']]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">'c'</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := \"\"]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">\"\"</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := {}]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">{}</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := y()]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">y()</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := List()]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">List ()</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := Set{}]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">Set {}</property>\n"
              "</properties>"
            );
  dump_okay ( "Struct [root := Struct [k := Map[]]]"
            , "<properties name=\"root\">\n"
              "  <property key=\"k\">Map []</property>\n"
              "</properties>"
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

  dump_throw_plain ("[]");
  dump_throw_plain ("true");
  dump_throw_plain ("0");
  dump_throw_plain ("0U");
  dump_throw_plain ("0L");
  dump_throw_plain ("0UL");
  dump_throw_plain ("0.0f");
  dump_throw_plain ("0.0");
  dump_throw_plain ("'c'");
  dump_throw_plain ("\"\"");
  dump_throw_plain ("{}");
  dump_throw_plain ("y()");
  dump_throw_plain ("List()");
  dump_throw_plain ("Set{}");
  dump_throw_plain ("Map[]");

  dump_throw_depth ("Struct[k:=0]", "k", "0");
  dump_throw_depth ("Struct[k:=0, Struct:=[]]", "k", "0");
  dump_throw_depth ("Struct[deeper:=Struct[], k:=0]", "k", "0");
}

namespace
{
  using position_type = std::pair< std::list<std::string>
                                 , gspc::pnet::type::value::value_type
                                 >;
}

GSPC_BOOST_TEST_LOG_VALUE_PRINTER (position_type, os, position)
{
  os << "Position " << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (position.first)
     << ": " << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (position.second);
}

BOOST_AUTO_TEST_CASE (positions_works)
{
  using gspc::pnet::type::value::value_type;
  using gspc::pnet::type::value::path::split;

#define LITERAL(literal...)                                             \
  do                                                                    \
  {                                                                     \
    std::list<position_type> positions;                                 \
    positions.push_back                                                 \
      (std::make_pair (std::list<std::string>(), literal));             \
    BOOST_REQUIRE_EQUAL                                                 \
      (gspc::pnet::type::value::positions (literal), positions);              \
  } while (false)

  LITERAL (gspc::we::type::literal::control());
  LITERAL (true);
  LITERAL (0);
  LITERAL (0L);
  LITERAL (0U);
  LITERAL (0UL);
  LITERAL (0.0f);
  LITERAL (0.0);
  LITERAL ('c');
  LITERAL (std::string (""));
  LITERAL (gspc::pnet::type::bitsetofint::type());
  LITERAL (gspc::we::type::bytearray());
  LITERAL (std::list<value_type>());
  LITERAL (std::set<value_type>());
  LITERAL (std::map<value_type, value_type>());

#undef LITERAL

  std::list<position_type> positions;
  value_type value;

  gspc::pnet::type::value::poke ("a", value, 0);
  positions.push_back (std::make_pair (split ("a"), 0));

  BOOST_REQUIRE_EQUAL (gspc::pnet::type::value::positions (value), positions);

  gspc::pnet::type::value::poke ("b", value, 0L);
  positions.push_back (std::make_pair (split ("b"), 0L));

  BOOST_REQUIRE_EQUAL (gspc::pnet::type::value::positions (value), positions);

  gspc::pnet::type::value::poke ("c.1", value, std::string ("foo"));
  positions.push_back (std::make_pair (split ("c.1"), std::string ("foo")));
  gspc::pnet::type::value::poke ("c.2", value, std::string ("bar"));
  positions.push_back (std::make_pair (split ("c.2"), std::string ("bar")));

  BOOST_REQUIRE_EQUAL (gspc::pnet::type::value::positions (value), positions);
}
