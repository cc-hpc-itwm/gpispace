// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>
#include <we/type/literal/function.hpp>
#include <we/type/value/function.hpp>
#include <we/type/value/show.hpp>

#include <iostream>
#include <string>

int
main (int argc, char ** argv)
{
  expr::eval::context context;

  context.bind ("a" , (argc > 1) ? atol (argv[1]) : 23);

  const std::string input
    ("${b} := (${a} := (if ${a} % 2 == 0 then ${a} div 2 else 3*${a}+1 endif)) == 1");

  expr::parse::parser parser (input);

  std::cout << "collatz sequence for " << context.value ("a") << ":" << std::endl;

  do
    {
      parser.eval_all (context);
      std::cout << " " << context.value ("a");
    }
  while (!value::function::is_true (context.value ("b")));

  std::cout << std::endl;

  return EXIT_SUCCESS;
}
