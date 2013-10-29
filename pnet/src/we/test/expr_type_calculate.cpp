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

  void TYPE_ERROR ( resolver_map_type& m
                  , std::string const& p
                  , std::string const& what
                  )
  {
    EXCEPTION<pnet::exception::type_error> ( m
                                           , p
                                           , "pnet::exception::type_error"
                                           , "type error: " + what
                                           );
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

  TYPE_ERROR ( m
             , "substr (\"\", ${a})"
             , "'substr' for types 'string' and 'FOO'"
               ", expected are types 'string' and 'long'"
             );

  m[path ("a")] = std::string ("long");

  OKAY (m, "substr (\"\", ${a})", std::string ("string"));

  TYPE_ERROR ( m
             , "substr (1L, ${a})"
             , "'substr' for types 'long' and 'long'"
               ", expected are types 'string' and 'long'"
             );
}

BOOST_AUTO_TEST_CASE (bitset_insert)
{
  resolver_map_type m;

  m[path ("a")] = std::string ("FOO");

  TYPE_ERROR ( m
             , "bitset_insert ({}, ${a})"
             , "'bitset_insert' for types 'bitset' and 'FOO'"
               ", expected are types 'bitset' and 'long'"
             );

  m[path ("a")] = std::string ("long");

  OKAY (m, "bitset_insert ({}, ${a})", std::string ("bitset"));
}

BOOST_AUTO_TEST_CASE (bitset_delete)
{
  resolver_map_type m;

  m[path ("a")] = std::string ("FOO");

  TYPE_ERROR ( m
             , "bitset_delete ({}, ${a})"
             , "'bitset_delete' for types 'bitset' and 'FOO'"
               ", expected are types 'bitset' and 'long'"
             );

  m[path ("a")] = std::string ("long");

  OKAY (m, "bitset_delete ({}, ${a})", std::string ("bitset"));
}

BOOST_AUTO_TEST_CASE (bitset_is_element)
{
  resolver_map_type m;

  m[path ("a")] = std::string ("FOO");

  TYPE_ERROR ( m
             , "bitset_is_element ({}, ${a})"
             , "'bitset_is_element' for types 'bitset' and 'FOO'"
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

  TYPE_ERROR ( m
             , "${a} || ${b}"
             , "' || ' for types 'FOO' and 'BAR'"
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

  TYPE_ERROR ( m
             , "${a} && ${b}"
             , "' && ' for types 'FOO' and 'BAR'"
               ", expected are types 'bool' and 'bool'"
             );

  m[path ("a")] = std::string ("bool");
  m[path ("b")] = std::string ("bool");

  OKAY (m, "${a} && ${b}", std::string ("bool"));
}

BOOST_AUTO_TEST_CASE (min)
{
  std::string const exp ("min (${a}, ${b})");

  resolver_map_type m;

  m[path ("a")] = std::string ("A");
  m[path ("b")] = std::string ("B");

  TYPE_ERROR (m, exp, "'min' for unequal types 'A' and 'B'");

  m[path ("b")] = std::string ("A");

  TYPE_ERROR ( m
             , exp
             , "'min' for type 'A', expected one of 'bool', 'char', 'string', 'int', 'unsigned int', 'long', 'unsigned long', 'float', 'double'"
             );

  m[path ("a")] = std::string ("bool");
  m[path ("b")] = std::string ("bool");

  OKAY (m, exp, std::string ("bool"));

  m[path ("a")] = std::string ("char");
  m[path ("b")] = std::string ("char");

  OKAY (m, exp, std::string ("char"));

  m[path ("a")] = std::string ("string");
  m[path ("b")] = std::string ("string");

  OKAY (m, exp, std::string ("string"));

  m[path ("a")] = std::string ("int");
  m[path ("b")] = std::string ("int");

  OKAY (m, exp, std::string ("int"));

  m[path ("a")] = std::string ("unsigned int");
  m[path ("b")] = std::string ("unsigned int");

  OKAY (m, exp, std::string ("unsigned int"));

  m[path ("a")] = std::string ("long");
  m[path ("b")] = std::string ("long");

  OKAY (m, exp, std::string ("long"));

  m[path ("a")] = std::string ("unsigned long");
  m[path ("b")] = std::string ("unsigned long");

  OKAY (m, exp, std::string ("unsigned long"));

  m[path ("a")] = std::string ("float");
  m[path ("b")] = std::string ("float");

  OKAY (m, exp, std::string ("float"));

  m[path ("a")] = std::string ("double");
  m[path ("b")] = std::string ("double");

  OKAY (m, exp, std::string ("double"));
}
