// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/parser.hpp>

#include <we/util/read.hpp>

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
    typedef expr::eval::context<std::string,double> context_t;
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
                expr::parse::parser<std::string,double> parser (input);
              
                while (!parser.empty())
                  {
                    cout << "parsed expression: " << parser.expr() << endl;

                    try
                      {
                        cout << "evaluated value: " << parser.eval (context)
                             << endl;
                      }
                    catch (expr::eval::missing_binding<std::string> e)
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
    typedef int ref_t;
    typedef expr::parse::parser<ref_t, double, read_int<int> > parser_t;
    typedef expr::eval::context<ref_t,double> context_t;

    {
      const unsigned int round (1000);
      const unsigned int max (1000);
      const std::string input ("${0} < ${1}");

      {
        Timer_t timer ("parse<int> once, eval often", max * round);

        context_t context;

        context.bind(1,max);
      
        parser_t parser (input);

        for (unsigned int r (0); r < round; ++r)
          {
            unsigned int i (0);

            do
              context.bind (0,i++);
            while (parser.eval_bool (context));
          }
      }

      {
        Timer_t timer ("often parse<int> and eval", max * round);

        context_t context;

        context.bind(1,max);

        for (unsigned int r (0); r < round; ++r)
          {
            unsigned int i (0);

            do
              context.bind (0,i++);
            while (parser_t (input, context).get_bool ());
          }
      }
    }
  }

  {
    typedef std::string ref_t;
    typedef expr::parse::parser<ref_t, double> parser_t;
    typedef expr::eval::context<ref_t,double> context_t;

    {
      const unsigned int round (1000);
      const unsigned int max (1000);
      const std::string input ("${0} < ${1}");

      {
        Timer_t timer ("parse<string> once, eval often", max * round);

        context_t context;

        context.bind("1",max);
      
        parser_t parser (input);

        for (unsigned int r (0); r < round; ++r)
          {
            unsigned int i (0);

            do
              context.bind ("0",i++);
            while (parser.eval_bool (context));
          }
      }

      {
        Timer_t timer ("often parse<string> and eval", max * round);

        context_t context;

        context.bind("1",max);

        for (unsigned int r (0); r < round; ++r)
          {
            unsigned int i (0);

            do
              context.bind ("0",i++);
            while (parser_t (input, context).get_bool ());
          }
      }
    }
  }

  malloc_stats();

  return EXIT_SUCCESS;
}
