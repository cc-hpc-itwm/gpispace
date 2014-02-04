// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_expr_get_names
#include <boost/test/unit_test.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/expr/parse/util/get_names.hpp>

#include <fhg/util/boost/test/printer/boost_unordered_set.hpp>
#include <fhg/util/boost/test/printer/list.hpp>
#include <fhg/util/split.hpp>

BOOST_AUTO_TEST_CASE (no_names)
{
  boost::unordered_set<std::list<std::string> > empty;

  BOOST_REQUIRE_EQUAL
    (empty, expr::parse::util::get_names (expr::parse::parser ("").front()));
  BOOST_REQUIRE_EQUAL
    (empty, expr::parse::util::get_names (expr::parse::parser ("1").front()));
 BOOST_REQUIRE_EQUAL
    ( empty
    , expr::parse::util::get_names (expr::parse::parser ("1 + 2").front())
    );
}

namespace
{
  std::list<std::string> split (const std::string& path)
  {
    return fhg::util::split< std::string
                             , std::list<std::string>
                             > (path, '.');
  }
}

BOOST_AUTO_TEST_CASE (one)
{
  boost::unordered_set<std::list<std::string> > names;
  names.insert (split ("x"));

  std::string const input ("${x}");

  BOOST_REQUIRE_EQUAL
    ( names
    , expr::parse::util::get_names (expr::parse::parser (input).front())
    );
}

BOOST_AUTO_TEST_CASE (two)
{
  boost::unordered_set<std::list<std::string> > names;
  names.insert (split ("x"));
  names.insert (split ("y"));

  std::string const input ("${x} + ${y}");

  BOOST_REQUIRE_EQUAL
    ( names
    , expr::parse::util::get_names (expr::parse::parser (input).front())
    );
}

BOOST_AUTO_TEST_CASE (duplicate_reported_once)
{
  boost::unordered_set<std::list<std::string> > names;
  names.insert (split ("x"));
  names.insert (split ("y"));

  std::string const input ("${x} + ${y} + ${x}");

  BOOST_REQUIRE_EQUAL
    ( names
    , expr::parse::util::get_names (expr::parse::parser (input).front())
    );
}

BOOST_AUTO_TEST_CASE (some_names)
{
  boost::unordered_set<std::list<std::string> > names;
  names.insert (split ("x.a"));
  names.insert (split ("y.a"));
  names.insert (split ("z.coord.phi"));
  names.insert (split ("x.b"));
  names.insert (split ("t"));

  std::string const input
    ("${x.a} * ${y.a} - ${z.coord.phi} + ${x.b} / sin (${t} + pi)");

  BOOST_REQUIRE_EQUAL
    ( names
    , expr::parse::util::get_names (expr::parse::parser (input).front())
    );
}
