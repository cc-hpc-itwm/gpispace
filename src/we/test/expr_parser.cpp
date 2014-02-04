// mirko.rahn@itwm.fraunhofer.de

#include <we/exception.hpp>
#include <we/expr/eval/context.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/type/value.hpp>
#include <we/type/value/show.hpp>

#include <sys/time.h>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

struct Timer_t
{
private:
  double t;
  const std::string msg;
  const unsigned int k;
  std::ostream & s;

  double current_time()
  {
    struct timeval tv;

    gettimeofday (&tv, NULL);

    return (double(tv.tv_sec) + double (tv.tv_usec) * 1E-6);
  }

public:
  explicit Timer_t ( const std::string & _msg
                   , const unsigned int & _k = 1
                   , std::ostream & _s = std::cout
                   )
    : t(-current_time())
    , msg(_msg)
    , k(_k)
    , s(_s)
  {}

  ~Timer_t ()
  {
    t += current_time();

    s << "time " << msg
      << " [" << k << "]: "
      << t
      << " [" << t / double(k) << "]"
      << " [" << double(k) / t << "]"
      << std::endl;
  }
};

using std::cin;
using std::cout;
using std::endl;

int main (int ac, char **)
{
  cout << "measure..." << endl;

  {
    typedef expr::parse::parser parser_t;
    typedef expr::eval::context context_t;

    {
      const long round (1000);
      const long max (1000);
      const std::string input ("${a} < ${b}");

      {
        Timer_t timer ("parse<string> once, eval often", max * round);

        context_t context;

        context.bind("b", max);

        parser_t parser (input);

        for (int r (0); r < round; ++r)
          {
            long i (0);

            do
              context.bind ("a", i++);
            while (parser.eval_front_bool (context));
          }
      }

      {
        Timer_t timer ("often parse<string> and eval", max * round);

        context_t context;

        context.bind("b", max);

        for (int r (0); r < round; ++r)
          {
            long i (0);

            do
              context.bind ("a", i++);
            while (parser_t (input, context).get_front_bool ());
          }
      }
    }
  }

  {
    typedef expr::parse::parser parser_t;
    typedef expr::eval::context context_t;

    std::ostringstream ss;

    ss << "${x} := ${x} + 1L;" << endl;
    ss << "${y} := double (${x}) / double (4L);" << endl;
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
    context.bind("x", 0L);
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

  return EXIT_SUCCESS;
}
