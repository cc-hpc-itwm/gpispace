// mirko.rahn@itwm.fraunhofer.de

#include <expr/parse/parser.hpp>

#include <util/timer.hpp>

#include <iostream>

#include <cstdlib>

using std::cin;
using std::cout;
using std::endl;

int main (void)
{
//   std::string input 
//     ("4*f(2) + --1 - 4*(-13+25/${j}) + c(4,8) <= 4*(f(2)+--1) - 4*-13 + 25 / ${ii}");

  {
    expr::eval::context<double> context;
    std::string input;

    while (getline(cin, input).good())
      switch (input[0])
        {
        case '#':
          context.clear();
          cout << "context deleted" << endl;
          break;
        case ':':
          {
            std::string::const_iterator pos (input.begin());
            const std::string::const_iterator end (input.end());

            ++pos;

            while (pos != end && isspace(*pos))
              ++pos;

            std::string name;

            while (pos != end && *pos != '=' && !isspace(*pos))
              {
                name.push_back (*pos);
                ++pos;
              }

            while (pos != end && isspace(*pos))
              ++pos;

            if (*pos != '=')
              cout << "parse error: syntax: name = value" << endl;

            ++pos;

            while (pos != end && isspace(*pos))
              ++pos;

            int value(0);

            while (isdigit(*pos))
              {
                value *= 10;
                value += *pos - '0';
                ++pos;
              }

            cout << "bind: " << name << " = " << value << endl;

            context.bind (name, value);
          }
          break;
        default:
          try
            {
              expr::parse::parser<double> parser (input);
              
              cout << "parsed expression: " << parser.expr() << endl;

              try
                {
                  cout << "evaluated value: " << parser.eval (context) << endl;
                }
              catch (expr::eval::missing_binding e)
                {
                  cout << e.what() << endl;
                }
            }
          catch (expr::exception e)
            {
              cout << input << endl;
              unsigned int k (e.eaten);
              while (k-->0)
                cout << " ";
              cout << "^" << endl;
              cout << e.what() << endl;
            }
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

  return EXIT_SUCCESS;
}
