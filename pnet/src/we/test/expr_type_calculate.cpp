// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE expr_type_calculate
#include <boost/test/unit_test.hpp>

#include <we/expr/type/calculate.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/exception.hpp>

#include <fhg/util/join.hpp>
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

  typedef boost::unordered_map< std::list<std::string>
                              , pnet::type::signature::signature_type
                              > resolver_map_type;
  class resolver
  {
  public:
    resolver (resolver_map_type const& m)
      : _m (m)
    {}

    pnet::type::signature::signature_type const&
      operator() (std::list<std::string> const& path) const
    {
      resolver_map_type::const_iterator const pos (_m.find (path));

      if (pos == _m.end())
      {
        throw std::runtime_error
          ( ( boost::format ("Could not resolve '%1%'")
            % fhg::util::join (path, ".")
            ).str()
          );
      }

      return pos->second;
    }

  private:
    resolver_map_type const& _m;
  };

  template<typename Ex>
    void CHECK_EXCEPTION ( resolver_map_type const& m
                         , ::expr::parse::parser const& p
                         , std::string const& name
                         , std::string const& what
                         )
  {
    try
    {
      pnet::expr::type::calculate (resolver (m), p.front());

      BOOST_FAIL (boost::format ("missing exception '%1%'") % name);
    }
    catch (Ex const& e)
    {
      BOOST_CHECK_EQUAL (e.what(), what);
    }
  }

  template<typename Ex>
    void CHECK_EXCEPTION ( resolver_map_type const& m
                         , std::string const& p
                         , std::string const& name
                         , std::string const& what
                         )
  {
    CHECK_EXCEPTION<Ex> (m, ::expr::parse::parser (p), name, what);
  }

  void CHECK_OKAY  ( resolver_map_type const& m
                   , std::string const& p
                   , pnet::type::signature::signature_type const& s
                   )
  {
    BOOST_CHECK
      ( s ==  pnet::expr::type::calculate ( resolver (m)
                                          , ::expr::parse::parser (p).front()
                                          )
      );
  }
}

BOOST_AUTO_TEST_CASE (substr)
{
  resolver_map_type m;

  CHECK_EXCEPTION<std::runtime_error>
    ( m
    , "substr (\"\", ${a})"
    , "std::runtime_error"
    , "Could not resolve 'a'"
    );

  m[path ("a")] = std::string ("FOO");

  CHECK_EXCEPTION<pnet::exception::type_error>
    ( m
    , "substr (\"\", ${a})"
    , "pnet::exception::type_error"
    , "type error: substr for types 'string' and 'FOO'"
    );

  m[path ("a")] = std::string ("long");

  CHECK_OKAY (m, "substr (\"\", ${a})", std::string ("string"));

  CHECK_EXCEPTION<pnet::exception::type_error>
    ( m
    , "substr (1L, ${a})"
    , "pnet::exception::type_error"
    , "type error: substr for types 'long' and 'long'"
    );
}

BOOST_AUTO_TEST_CASE (bitset_insert)
{
  resolver_map_type m;

  m[path ("a")] = std::string ("FOO");

  CHECK_EXCEPTION<pnet::exception::type_error>
    ( m
    , "bitset_insert ({}, ${a})"
    , "pnet::exception::type_error"
    , "type error: bitset_insert for types 'bitset' and 'FOO'"
    );

  m[path ("a")] = std::string ("long");

  CHECK_OKAY (m, "bitset_insert ({}, ${a})", std::string ("bitset"));
}

BOOST_AUTO_TEST_CASE (bitset_delete)
{
  resolver_map_type m;

  m[path ("a")] = std::string ("FOO");

  CHECK_EXCEPTION<pnet::exception::type_error>
    ( m
    , "bitset_delete ({}, ${a})"
    , "pnet::exception::type_error"
    , "type error: bitset_delete for types 'bitset' and 'FOO'"
    );

  m[path ("a")] = std::string ("long");

  CHECK_OKAY (m, "bitset_delete ({}, ${a})", std::string ("bitset"));
}

BOOST_AUTO_TEST_CASE (bitset_is_element)
{
  resolver_map_type m;

  m[path ("a")] = std::string ("FOO");

  CHECK_EXCEPTION<pnet::exception::type_error>
    ( m
    , "bitset_is_element ({}, ${a})"
    , "pnet::exception::type_error"
    , "type error: bitset_is_element for types 'bitset' and 'FOO'"
    );

  m[path ("a")] = std::string ("long");

  CHECK_OKAY (m, "bitset_is_element ({}, ${a})", std::string ("bool"));
}
