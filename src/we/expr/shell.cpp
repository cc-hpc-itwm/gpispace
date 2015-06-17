// mirko.rahn@itwm.fraunhofer.de

#include <we/exception.hpp>
#include <we/expr/eval/context.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/type/value/show.hpp>

#include <util-generic/print_exception.hpp>

#include <readline/history.h>
#include <readline/readline.h>

#include <string>

namespace
{
  bool read_line (std::string& s)
  {
    char* line (readline ("> "));

    if (!line)
    {
      return false;
    }

    s = std::string (line);

    if (*line)
    {
      add_history (line);
    }

    free (line);

    return true;
  }
}

int main()
try
{
  std::cout << "clear context: #" << std::endl
            << "list state: ?" << std::endl
            << "switch constant folding: g" << std::endl;

  expr::eval::context context;
  std::string input;
  bool constant_folding (true);

  while (read_line (input))
  {
    switch (input[0])
    {
    case '?':
      std::cout << "constant_folding (switch with g): "
                << std::boolalpha << constant_folding << std::noboolalpha
                << std::endl
                << "context (delete with #): "
                << std::endl << context;
      break;
    case 'g':
    case 'G':
      constant_folding = not constant_folding;

      std::cout << "constant_folding: "
                << std::boolalpha << constant_folding << std::noboolalpha
                << std::endl;
      break;
    case '#':
      context = expr::eval::context();
      std::cout << "context deleted" << std::endl;
      break;
    default:
      try
      {
        expr::parse::parser parser (input, constant_folding);

        while (!parser.empty())
        {
          std::cout << "expression: " << parser.front() << std::endl;

          try
          {
            std::cout << "evals to: "
                      << pnet::type::value::show (parser.eval_front (context))
                      << std::endl;
          }
          catch (const pnet::exception::missing_binding & e)
          {
            std::cout << e.what() << std::endl;
          }

          parser.pop_front();
        }
      }
      catch (expr::exception::eval::divide_by_zero e)
      {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
      }
      catch (expr::exception::eval::type_error e)
      {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
      }
      catch (expr::exception::parse::exception e)
      {
        std::cout << input << std::endl;
        for (unsigned int k (0); k < e.eaten; ++k)
          std::cout << " ";
        std::cout << "^" << std::endl;
        std::cout << "EXCEPTION: " << e.what() << std::endl;
      }
      catch (std::runtime_error e)
      {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
      }
    }
  }

  std::cout << std::endl;

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';

  return EXIT_FAILURE;
}
