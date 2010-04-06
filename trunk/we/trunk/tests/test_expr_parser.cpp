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
  if (ac > 1) { // just assume "-i" as first parameter for interactive mode
    cout << "enter expression, ^D to start measurement" << endl;
    cout << "clear context: #" << endl;
    cout << "list context: ?" << endl;
    typedef expr::eval::context<std::string> context_t;
    context_t context;
    std::string input;

    const std::string prompt ("> ");

    cout << prompt;

    while (getline(cin, input).good())
      {
        switch (input[0])
          {
          case '?': cout << context; break;
          case '#': context.clear(); cout << "context deleted" << endl; break;
          default:
            try
              {
                typedef expr::parse::parser<std::string> parser_t;

                parser_t parser (input);
              
                while (!parser.empty())
                  {
                    cout << "expression: " << parser.front() << endl;

                    try
                      {
                        cout 
                          << "evals to: "
                          << expr::variant::show (parser.eval_front (context))
                          << endl;
                      }
                    catch (expr::eval::missing_binding<std::string> e)
                      {
                        cout << e.what() << endl;
                      }

                    parser.pop_front();
                  }
              }
            catch (expr::not_integral e)
              {
                cout << "EXCEPTION: " << e.what() << endl;
              }
            catch (expr::divide_by_zero e)
              {
                cout << "EXCEPTION: " << e.what() << endl;
              }
            catch (expr::exception e)
              {
                cout << input << endl;
                for (unsigned int k (0); k < e.eaten; ++k)
                  cout << " ";
                cout << "^" << endl;
                cout << "EXCEPTION: " << e.what() << endl;
              }
          }
        cout << prompt;
      }
  }

  cout << "measure..." << endl;

  {
    typedef int ref_t;
    typedef expr::parse::parser<ref_t, util::read_int<int> > parser_t;
    typedef expr::eval::context<ref_t> context_t;
    {
      const long round (1000);
      const long max (1000);
      const std::string input ("${0} < ${1}");

      {
        Timer_t timer ("parse<int> once, eval often", max * round);

        context_t context;

        context.bind(1,max);
      
        parser_t parser (input);

        for (unsigned int r (0); r < round; ++r)
          {
            long i (0);

            do
              context.bind (0,i++);
            while (parser.eval_front_bool (context));
          }
      }

      {
        Timer_t timer ("often parse<int> and eval", max * round);

        context_t context;

        context.bind(1,max);

        for (unsigned int r (0); r < round; ++r)
          {
            long i (0);

            do
              context.bind (0,i++);
            while (parser_t (input, context).get_front_bool ());
          }
      }
    }
  }

  {
    typedef std::string ref_t;
    typedef expr::parse::parser<ref_t> parser_t;
    typedef expr::eval::context<ref_t> context_t;

    {
      const long round (1000);
      const long max (1000);
      const std::string input ("${0} < ${1}");

      {
        Timer_t timer ("parse<string> once, eval often", max * round);

        context_t context;

        context.bind("1",max);
      
        parser_t parser (input);

        for (unsigned int r (0); r < round; ++r)
          {
            long i (0);

            do
              context.bind ("0",i++);
            while (parser.eval_front_bool (context));
          }
      }

      {
        Timer_t timer ("often parse<string> and eval", max * round);

        context_t context;

        context.bind("1",max);

        for (unsigned int r (0); r < round; ++r)
          {
            long i (0);

            do
              context.bind ("0",i++);
            while (parser_t (input, context).get_front_bool ());
          }
      }
    }
  }

  {
    typedef std::string ref_t;
    typedef expr::parse::parser<ref_t> parser_t;
    typedef expr::eval::context<ref_t> context_t;

    std::ostringstream ss;

    ss << "${x} := ${x} + 1;" << endl;
    ss << "${y} := ${x} / 4;" << endl;
    ss << "${ceil} := ceil(${y});" << endl;
    ss << "${floor} := floor${y} /* note the omision of parens */;" << endl;
    ss << "${round_half_up} := floor(${y} + 0.5/*comment, /* NESTED */*/);" << endl;
    ss << "${round_half_down} := ceil(${y} - 0.5);" << endl;
    ss << "${round} := round(${y});" << endl;
    ss << "/* round switches between half_up and half_down */" << endl;
    ss << "round(2.5); round(2.5); round(2.5); round(2.5);" << endl;

    const std::string input (ss.str());

    cout << "INPUT:" << endl << input << endl;

    context_t context;
    context.bind("x",0L);
    parser_t parser (input);

    cout << "PARSED:" << endl << parser;
    cout << "EVAL_ALL:" << endl;

    int i = 12;

    while (i-->0)
      {
        parser.eval_all (context);
        cout << context << endl;
      }
  }

  malloc_stats();

  return EXIT_SUCCESS;
}
