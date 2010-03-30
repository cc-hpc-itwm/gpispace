// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/parser.hpp>

#include <iostream>
#include <string>

int
main (int argc, char ** argv)
{
  expr::eval::context<int, double> context;

  context.bind (0 , (argc > 1) ? atof (argv[1]) : 23);
  
  const std::string input
    ("${1} := (${0} := if ${0} % 2 == 0 then ${0}/2 else 3*${0}+1 endif) - 1");

  expr::parse::parser<int, double> parser (input);

  std::cout << "collatz sequence for " << context.value (0) << ":" << std::endl;

  do
    {
      parser.eval_all (context);
      std::cout << " " << context.value (0);
    }
  while (!expr::token::function::is_zero (context.value (1)));

  std::cout << std::endl;

  return EXIT_SUCCESS;
}
