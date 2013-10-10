// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE expr_type_calculate
#include <boost/test/unit_test.hpp>

#include <we/expr/type/calculate.hpp>

#include <we/expr/parse/parser.hpp>

namespace
{
  boost::optional<pnet::type::signature::signature_type>
  resolve_nothing (const std::string&)
  {
    return boost::none;
  }
}

BOOST_AUTO_TEST_CASE (value_assignment)
{
  {
    ::expr::parse::parser p ("${a} := Set {}");

    pnet::expr::type::calculate (resolve_nothing, p.front());
  }

  {
    ::expr::parse::parser p ("0L + ${b}");

    pnet::expr::type::calculate (resolve_nothing, p.front());
  }
}
