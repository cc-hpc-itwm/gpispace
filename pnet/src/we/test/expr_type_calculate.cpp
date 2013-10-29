// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE expr_type_calculate
#include <boost/test/unit_test.hpp>

#include <we/expr/type/calculate.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/type/value/name.hpp>
#include <we/exception.hpp>

#include <fhg/util/split.hpp>

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

#include <stdexcept>

namespace
{
  std::list<std::string> init_tnames()
  {
    std::list<std::string> tn (pnet::type::value::type_names());

    tn.push_back ("UNKNOWN_TYPE");

    return tn;
  }
  std::list<std::string> tnames()
  {
    static std::list<std::string> tn (init_tnames());

    return tn;
  }

  std::list<std::string> path (std::string const& s)
  {
    return fhg::util::split< std::string
                           , std::list<std::string>
                           > (s, '.');
  }

  template<typename Ex>
    void EXCEPTION ( pnet::expr::type::resolver_map_type& m
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
    void EXCEPTION ( pnet::expr::type::resolver_map_type& m
                   , std::string const& p
                   , std::string const& name
                   , std::string const& what
                   )
  {
    EXCEPTION<Ex> (m, ::expr::parse::parser (p), name, what);
  }

  void TYPE_ERROR ( pnet::expr::type::resolver_map_type& m
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

  void OKAY  ( pnet::expr::type::resolver_map_type& m
             , std::string const& p
             , pnet::type::signature::signature_type const& s
             )
  {
    BOOST_CHECK
      (s == pnet::expr::type::calculate (m, ::expr::parse::parser (p).front()));
  }

  void BIN_REQUIRE ( std::string const& exp
                   , std::string const& okay_l
                   , std::string const& okay_r
                   , std::string const& result
                   , std::string const& error
                   )
  {
    pnet::expr::type::resolver_map_type m;

    BOOST_FOREACH (std::string const& l, tnames())
    {
      BOOST_FOREACH (std::string const& r, tnames())
      {
        m[path ("a")] = l;
        m[path ("b")] = r;

        if (l == okay_l && r == okay_r)
        {
          OKAY (m, exp, result);
        }
        else
        {
          TYPE_ERROR (m, exp, (boost::format (error) % l % r).str());
        }
      }
    }
  }

  void BIN_EQUAL ( std::string const& exp
                 , std::string const& token
                 )
  {
    pnet::expr::type::resolver_map_type m;

    BOOST_FOREACH (std::string const& l, tnames())
    {
      BOOST_FOREACH (std::string const& r, tnames())
      {
        if (l != r)
        {
          m[path ("a")] = l;
          m[path ("b")] = r;

          TYPE_ERROR
            ( m
            , exp
            , ( boost::format ("'%1%' for unequal types '%2%' and '%3%'")
              % token
              % l
              % r
              ).str()
            );
        }
      }
    }
  }

  class BINEQ_ONE_OF
  {
  public:
    BINEQ_ONE_OF (std::string const& exp, std::string const& token)
      : _exp (exp)
      , _token (token)
      , _count (0)
      , _allowed()
      , _expected()
    {}
    BINEQ_ONE_OF& allow (std::string const& t)
    {
      _allowed.insert (t);

      if (_count > 0)
      {
        _expected << ", ";
      }
      _expected << "'" << t << "'";

      ++_count;

      return *this;
    }
    void check() const
    {
      pnet::expr::type::resolver_map_type m;

      BOOST_FOREACH (std::string const& t, tnames())
      {
        m[path ("a")] = t;
        m[path ("b")] = t;

        if (_allowed.count (t))
        {
          OKAY (m, _exp, t);
        }
        else
        {
          TYPE_ERROR
            ( m
            , _exp
            , ( boost::format ("'%1%' for type '%2%', expected %3%%4%")
              % _token
              % t
              % ((_count > 1) ? "one of " : "")
              % _expected.str()
              ).str()
            );
        }
      }
    }

  private:
    std::string const& _exp;
    std::string const& _token;
    unsigned int _count;
    std::set<std::string> _allowed;
    std::ostringstream _expected;
  };
}

BOOST_AUTO_TEST_CASE (lookup)
{
  pnet::expr::type::resolver_map_type m;

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
  BIN_REQUIRE ( "substr (${a}, ${b})"
              , "string"
              , "long"
              , "string"
              , "'substr' for types '%1%' and '%2%'"
              ", expected are types 'string' and 'long'"
              );
}

BOOST_AUTO_TEST_CASE (bitset_insert)
{
  BIN_REQUIRE ( "bitset_insert (${a}, ${b})"
              , "bitset"
              , "long"
              , "bitset"
              , "'bitset_insert' for types '%1%' and '%2%'"
                ", expected are types 'bitset' and 'long'"
              );
}

BOOST_AUTO_TEST_CASE (bitset_delete)
{
  BIN_REQUIRE ( "bitset_delete (${a}, ${b})"
              , "bitset"
              , "long"
              , "bitset"
              , "'bitset_delete' for types '%1%' and '%2%'"
                ", expected are types 'bitset' and 'long'"
              );
}

BOOST_AUTO_TEST_CASE (bitset_is_element)
{
  BIN_REQUIRE ( "bitset_is_element (${a}, ${b})"
              , "bitset"
              , "long"
              , "bool"
              , "'bitset_is_element' for types '%1%' and '%2%'"
                ", expected are types 'bitset' and 'long'"
              );
}

BOOST_AUTO_TEST_CASE (_or)
{
  BIN_REQUIRE ( "${a} || ${b}"
              , "bool"
              , "bool"
              , "bool"
              , "' || ' for types '%1%' and '%2%'"
                ", expected are types 'bool' and 'bool'"
              );
}

BOOST_AUTO_TEST_CASE (_and)
{
  BIN_REQUIRE ( "${a} && ${b}"
              , "bool"
              , "bool"
              , "bool"
              , "' && ' for types '%1%' and '%2%'"
                ", expected are types 'bool' and 'bool'"
              );
}

BOOST_AUTO_TEST_CASE (lt)
{
  std::string const exp ("${a} :lt: ${b}");

  BIN_EQUAL (exp, " < ");

  BINEQ_ONE_OF (exp, " < ")
    . allow ("bool")
    . allow ("char")
    . allow ("string")
    . allow ("int")
    . allow ("unsigned int")
    . allow ("long")
    . allow ("unsigned long")
    . allow ("float")
    . allow ("double")
    . check();
}

BOOST_AUTO_TEST_CASE (le)
{
  std::string const exp ("${a} :le: ${b}");

  BIN_EQUAL (exp, " <= ");

  BINEQ_ONE_OF (exp, " <= ")
    . allow ("bool")
    . allow ("char")
    . allow ("string")
    . allow ("int")
    . allow ("unsigned int")
    . allow ("long")
    . allow ("unsigned long")
    . allow ("float")
    . allow ("double")
    . check();
}

BOOST_AUTO_TEST_CASE (gt)
{
  std::string const exp ("${a} :gt: ${b}");

  BIN_EQUAL (exp, " > ");

  BINEQ_ONE_OF (exp, " > ")
    . allow ("bool")
    . allow ("char")
    . allow ("string")
    . allow ("int")
    . allow ("unsigned int")
    . allow ("long")
    . allow ("unsigned long")
    . allow ("float")
    . allow ("double")
    . check();
}

BOOST_AUTO_TEST_CASE (ge)
{
  std::string const exp ("${a} :ge: ${b}");

  BIN_EQUAL (exp, " >= ");

  BINEQ_ONE_OF (exp, " >= ")
    . allow ("bool")
    . allow ("char")
    . allow ("string")
    . allow ("int")
    . allow ("unsigned int")
    . allow ("long")
    . allow ("unsigned long")
    . allow ("float")
    . allow ("double")
    . check();
}

BOOST_AUTO_TEST_CASE (ne)
{
  std::string const exp ("${a} :ne: ${b}");

  BIN_EQUAL (exp, " != ");

  BINEQ_ONE_OF (exp, " != ")
    . allow ("bool")
    . allow ("char")
    . allow ("string")
    . allow ("int")
    . allow ("unsigned int")
    . allow ("long")
    . allow ("unsigned long")
    . allow ("float")
    . allow ("double")
    . check();
}

BOOST_AUTO_TEST_CASE (min)
{
  std::string const exp ("min (${a}, ${b})");

  BIN_EQUAL (exp, "min");

  BINEQ_ONE_OF (exp, "min")
    . allow ("bool")
    . allow ("char")
    . allow ("string")
    . allow ("int")
    . allow ("unsigned int")
    . allow ("long")
    . allow ("unsigned long")
    . allow ("float")
    . allow ("double")
    . check();
}

BOOST_AUTO_TEST_CASE (max)
{
  std::string const exp ("max (${a}, ${b})");

  BIN_EQUAL (exp, "max");

  BINEQ_ONE_OF (exp, "max")
    . allow ("bool")
    . allow ("char")
    . allow ("string")
    . allow ("int")
    . allow ("unsigned int")
    . allow ("long")
    . allow ("unsigned long")
    . allow ("float")
    . allow ("double")
    . check();
}
