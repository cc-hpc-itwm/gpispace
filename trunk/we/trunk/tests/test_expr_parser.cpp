// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/parser.hpp>

#include "timer.hpp"

#include <iostream>

#include <cstdlib>

#include <malloc.h>

using std::cin;
using std::cout;
using std::endl;

int main (int ac, char **)
{
//   std::string input 
//     ("4*f(2) + --1 - 4*(-13+25/${j}) + c(4,8) <= 4*(f(2)+--1) - 4*-13 + 25 / ${ii}");


  if (ac > 1) { // just assume "-i" as first parameter for interactive mode
    cout << "enter expression, ^D to start measurement" << endl;
    cout << "clear context: #" << endl;
    cout << "list context: ?" << endl;
    typedef expr::eval::context<double> context_t;
    context_t context;
    std::string input;

    const std::string prompt ("> ");

    cout << prompt;

    while (getline(cin, input).good())
      {
        switch (input[0])
          {
          case '?':
            for ( context_t::const_iterator it (context.begin())
                    ; it != context.end()
                    ; ++it
                )
              cout << it->first << " = " << it->second << endl;
            break;
          case '#':
            context.clear();
            cout << "context deleted" << endl;
            break;
          default:
            try
              {
                expr::parse::parser<double> parser (input);
              
                while (!parser.empty())
                  {
                    cout << "parsed expression: " << parser.expr() << endl;

                    try
                      {
                        cout << "evaluated value: " << parser.eval (context)
                             << endl;
                      }
                    catch (expr::eval::missing_binding e)
                      {
                        cout << e.what() << endl;
                      }

                    parser.pop();
                  }
              }
            catch (expr::exception e)
              {
                cout << input << endl;
                for (unsigned int k (0); k < e.eaten; ++k)
                  cout << " ";
                cout << "^" << endl;
                cout << e.what() << endl;
              }
          }
        cout << prompt;
      }
  }

  cout << "measure..." << endl;

  {
    const unsigned int round (1000);
    const unsigned int max (1000);
    const std::string input ("${i} < ${max}");

    {
      Timer_t timer ("parse once, evaluate often", max * round);

      expr::eval::context<double> context;

      context.bind("max",max);

      expr::parse::parser<double> parser (input);

      for (unsigned int r (0); r < round; ++r)
        {
          unsigned int i (0);

          do
            context.bind ("i",i++);
          while (parser.eval_bool (context));
        }
    }

    {
      Timer_t timer ("parse with evaluate often", max * round);

      expr::eval::context<double> context;

      context.bind("max",max);

      for (unsigned int r (0); r < round; ++r)
        {
          unsigned int i (0);

          do
            context.bind ("i",i++);
          while (expr::parse::parser<double>(input, context).get_bool ());
        }
    }
  }

  malloc_stats();

  return EXIT_SUCCESS;
}
