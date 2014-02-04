// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_expr_rename
#include <boost/test/unit_test.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <iostream>

BOOST_AUTO_TEST_CASE (NO_TEST)
{
  typedef expr::eval::context context_t;
  typedef expr::parse::parser parser_t;

  context_t context;
  std::ostringstream input;

  input << "${a} := ${b};" << std::endl;
  input << "${a.x} := ${a.y} + ${e} * sin (${a.z});" << std::endl;
  input << "${c} := ${b};" << std::endl;

  parser_t parser (input.str());

  std::cout << parser << std::endl;

  parser.rename ("a", "A");

  std::cout << parser << std::endl;
}
