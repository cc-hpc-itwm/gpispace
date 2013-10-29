// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE expr_type_calculate
#include <boost/test/unit_test.hpp>

#include <we/expr/type/calculate.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/exception.hpp>

#include <fhg/util/split.hpp>

#include <boost/format.hpp>
#include <boost/unordered_map.hpp>

#include <stdexcept>

namespace
{
  std::list<std::string> path (std::string const& s)
  {
    return fhg::util::split< std::string
                           , std::list<std::string>
                           > (s, '.');
  }

  using pnet::expr::type::resolver_map_type;

  template<typename Ex>
    void EXCEPTION ( resolver_map_type& m
                   , ::expr::parse::parser const& p
                   , std::string const& name
                   , std::string const& what
                   )
  {
    try
    {
      pnet::expr::type::calculate (m, p.front());

      BOOST_FAIL (boost::format ("missing exception '%1%'") % name);
    }
    catch (Ex const& e)
    {
      BOOST_CHECK_EQUAL (e.what(), what);
    }
  }

  template<typename Ex>
    void EXCEPTION ( resolver_map_type& m
                   , std::string const& p
                   , std::string const& name
                   , std::string const& what
                   )
  {
    EXCEPTION<Ex> (m, ::expr::parse::parser (p), name, what);
  }

  void OKAY  ( resolver_map_type& m
             , std::string const& p
             , pnet::type::signature::signature_type const& s
             )
  {
    BOOST_CHECK
      (s == pnet::expr::type::calculate (m, ::expr::parse::parser (p).front()));
  }
}

BOOST_AUTO_TEST_CASE (lookup)
{
  resolver_map_type m;

  EXCEPTION<std::runtime_error> ( m
                                , "${a}"
                                , "std::runtime_error"
                                , "Could not resolve 'a'"
                                );

  m[path ("a")] = std::string ("Foo");

  OKAY (m, "${a}", std::string ("Foo"));

  EXCEPTION<std::runtime_error> ( m
                                , "${a.a}"
                                , "std::runtime_error"
                                , "Could not resolve 'a.a'"
                                );

  m[path ("a.a")] = std::string ("Bar");

  OKAY (m, "${a.a}", std::string ("Bar"));

  BOOST_CHECK_EQUAL (m.size(), 2);
  BOOST_CHECK (m.find (path ("a")) != m.end());
  BOOST_CHECK (m.find (path ("a.a")) != m.end());
  BOOST_CHECK (  m.at (path ("a"))
              == pnet::type::signature::signature_type (std::string ("Foo"))
              );
  BOOST_CHECK (  m.at (path ("a.a"))
              == pnet::type::signature::signature_type (std::string ("Bar"))
              );
}

BOOST_AUTO_TEST_CASE (substr)
{
  resolver_map_type m;

  m[path ("a")] = std::string ("FOO");

  EXCEPTION<pnet::exception::type_error>
    ( m
    , "substr (\"\", ${a})"
    , "pnet::exception::type_error"
    , "type error: 'substr' for types 'string' and 'FOO'"
      ", expected are types 'string' and 'long'"
    );

  m[path ("a")] = std::string ("long");

  OKAY (m, "substr (\"\", ${a})", std::string ("string"));

  EXCEPTION<pnet::exception::type_error>
    ( m
    , "substr (1L, ${a})"
    , "pnet::exception::type_error"
    , "type error: 'substr' for types 'long' and 'long'"
      ", expected are types 'string' and 'long'"
    );
}

BOOST_AUTO_TEST_CASE (bitset_insert)
{
  resolver_map_type m;

  m[path ("a")] = std::string ("FOO");

  EXCEPTION<pnet::exception::type_error>
    ( m
    , "bitset_insert ({}, ${a})"
    , "pnet::exception::type_error"
    , "type error: 'bitset_insert' for types 'bitset' and 'FOO'"
      ", expected are types 'bitset' and 'long'"
    );

  m[path ("a")] = std::string ("long");

  OKAY (m, "bitset_insert ({}, ${a})", std::string ("bitset"));
}

BOOST_AUTO_TEST_CASE (bitset_delete)
{
  resolver_map_type m;

  m[path ("a")] = std::string ("FOO");

  EXCEPTION<pnet::exception::type_error>
    ( m
    , "bitset_delete ({}, ${a})"
    , "pnet::exception::type_error"
    , "type error: 'bitset_delete' for types 'bitset' and 'FOO'"
      ", expected are types 'bitset' and 'long'"
    );

  m[path ("a")] = std::string ("long");

  OKAY (m, "bitset_delete ({}, ${a})", std::string ("bitset"));
}

BOOST_AUTO_TEST_CASE (bitset_is_element)
{
  resolver_map_type m;

  m[path ("a")] = std::string ("FOO");

  EXCEPTION<pnet::exception::type_error>
    ( m
    , "bitset_is_element ({}, ${a})"
    , "pnet::exception::type_error"
    , "type error: 'bitset_is_element' for types 'bitset' and 'FOO'"
      ", expected are types 'bitset' and 'long'"
    );

  m[path ("a")] = std::string ("long");

  OKAY (m, "bitset_is_element ({}, ${a})", std::string ("bool"));
}

BOOST_AUTO_TEST_CASE (_or)
{
  resolver_map_type m;

  m[path ("a")] = std::string ("FOO");
  m[path ("b")] = std::string ("BAR");

  EXCEPTION<pnet::exception::type_error>
    ( m
    , "${a} || ${b}"
    , "pnet::exception::type_error"
    , "type error: ' || ' for types 'FOO' and 'BAR'"
      ", expected are types 'bool' and 'bool'"
    );

  m[path ("a")] = std::string ("bool");
  m[path ("b")] = std::string ("bool");

  OKAY (m, "${a} || ${b}", std::string ("bool"));
}

BOOST_AUTO_TEST_CASE (_and)
{
  resolver_map_type m;

  m[path ("a")] = std::string ("FOO");
  m[path ("b")] = std::string ("BAR");

  EXCEPTION<pnet::exception::type_error>
    ( m
    , "${a} && ${b}"
    , "pnet::exception::type_error"
    , "type error: ' && ' for types 'FOO' and 'BAR'"
      ", expected are types 'bool' and 'bool'"
    );

  m[path ("a")] = std::string ("bool");
  m[path ("b")] = std::string ("bool");

  OKAY (m, "${a} && ${b}", std::string ("bool"));
}
