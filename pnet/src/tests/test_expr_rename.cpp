// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include "timer.hpp"

#include <iostream>

#include <cstdlib>

#ifndef __APPLE__
// malloc.h is deprecated on OSX.
#include <malloc.h>
#endif

int main (int, char **)
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

  return EXIT_SUCCESS;
}
