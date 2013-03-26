// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/action.hpp>

#include <we/expr/token/assoc.hpp>
#include <we/expr/token/prec.hpp>
#include <we/expr/token/prop.hpp>
#include <we/expr/exception.hpp>

#include <fhg/util/show.hpp>

#include <iostream>

namespace expr
{
  namespace parse
  {
    namespace action
    {
      std::ostream& operator<< (std::ostream& s, const type& action)
      {
        switch (action)
          {
          case shift: return s << "shift";
          case reduce: return s << "reduce";
          case accept: return s << "accept";
          case error1: return s << "error: missing right parenthesis";
          case error2: return s << "error: missing operator";
          case error3: return s << "error: unbalanced parenthesis";
          case error4: return s << "error: invalid function argument";
          default: throw exception::strange ("action " + fhg::util::show(action));
          }
      }

      //! \todo change behavior of parser here
      //! \todo make a real table, avoid any conditional
      type action (const token::type& top, const token::type& inp)
      {
        if (top == token::lpr)
          {
            if (inp == token::eof)
              return error1;

            return shift;
          }

        if (top == token::rpr)
          {
            if (inp == token::lpr)
              return error2;

            if (inp == token::sep)
              return reduce;

            if (token::is_builtin (inp))
              return error3;

            return reduce;
          }

        if (top == token::eof)
          {
            if (inp == token::eof)
              return accept;

            if (inp == token::rpr)
              return error3;

            if (inp == token::sep)
              return error4;

            return shift;
          }

        if (top == token::sep)
          {
            if (inp == token::eof)
              return error4;

            return reduce;
          }

        if (token::is_builtin (top))
          {
            if (inp == token::lpr)
              return shift;

            return reduce;
          }

        //        assert (top < token::fac);

        if (inp == token::sep)
          return reduce;

        if (inp == token::lpr)
          return shift;

        if (inp == token::rpr)
          return reduce;

        if (inp == token::eof)
          return reduce;

        if (token::is_builtin (inp))
          return shift;

        //        assert (inp < token::fac);

        if (prec::prec (top) < prec::prec (inp))
          return shift;

        if (top == inp)
          {
            if (associativity::associativity (top) == associativity::left)
              return reduce;

            return shift;
          }

        if (top == token::_not && inp == token::neg)
          return shift;

        return reduce;
      }
    }
  }
}
