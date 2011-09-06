// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_ACTION_HPP
#define _EXPR_PARSE_ACTION_HPP

#include <we/expr/token/assoc.hpp>
#include <we/expr/token/prec.hpp>
#include <we/expr/token/prop.hpp>
#include <we/expr/token/type.hpp>
#include <we/expr/exception.hpp>

#include <fhg/util/show.hpp>

#include <iostream>

namespace expr
{
  namespace parse
  {
    namespace action
    {
      enum type
      { shift
      , reduce
      , accept
      , error1
      , error2
      , error3
      , error4
      };

      inline std::ostream & operator << (std::ostream & s, const type & action)
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

      inline type action (const token::type & top, const token::type & inp)
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

        if (inp == token::_if)
          return shift;

        if (top == token::_if)
          {
            if (inp == token::_then)
              return shift;

            return reduce;
          }

        if (top == token::_then)
          {
            if (inp == token::_else)
              return shift;

            return reduce;
          }

        if (top == token::_else)
          {
            if (inp == token::_endif)
              return reduce;

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

#endif
