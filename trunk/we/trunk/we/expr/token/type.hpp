// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_TYPE_HPP
#define _EXPR_TOKEN_TYPE_HPP

#include <we/expr/exception.hpp>

#include <stdexcept>
#include <iostream>

namespace expr
{
  namespace token
  {
    enum type
    { _or                     // prec  0, left associative
    , _and                    // prec  1, left associative
    , _not                    // prec 30, right associative
    , lt, le, gt, ge, ne, eq  // prec 10, left associative

    , add, sub                // prec 21, left associative
    , mul, div, divint        // prec 22, left associative
    , mod, modint             // prec 23, left associative
    , _pow                    // prec 24, right associative
    , _powint                 // prec 24, right associative
    , neg                     // prec 25, unary minus

    , min, max, abs
    , _floor, _ceil, _round
    , _sin, _cos
    , _sqrt, _log
    , _toint, _todouble
    , _len
    , _substr

    , sep                     // comma
    , lpr, rpr                // parenthesis

    , val                     // value
    , ref                     // reference to context

    , define                  // prec -99, left associative

    , _if, _then, _else, _endif, _ite

    , eof
    };

    static std::ostream & operator << (std::ostream & s, const type & token)
    {
      switch (token)
        {
        case _or: return s << " | ";
        case _and: return s << " & ";
        case _not: return s << "!";
        case lt: return s << " < ";
        case le: return s << " <= ";
        case gt: return s << " > ";
        case ge: return s << " >= ";
        case ne: return s << " != ";
        case eq: return s << " == ";
        case add: return s << " + ";
        case sub: return s << " - ";
        case mul: return s << " * ";
        case div: return s << " / ";
        case divint: return s << " div ";
        case mod: return s << " % ";
        case modint: return s << " mod ";
        case _pow: return s << "**";
        case _powint: return s << "^";
        case neg: return s << "-";
        case min: return s << "min";
        case max: return s << "max";
        case _floor: return s << "floor";
        case _ceil: return s << "ceil";
        case _round: return s << "round";
        case _sin: return s << "sin";
        case _cos: return s << "cos";
        case _sqrt: return s << "sqrt";
        case _log: return s << "log";
        case _toint: return s << "int";
        case _todouble: return s << "double";
        case abs: return s << "abs";
        case sep: return s << ", ";
        case lpr: return s << "(";
        case rpr: return s << ")";
        case val: return s << "<val>";
        case ref: return s << "<ref>";
        case eof: return s << "<eof>";
        case define: return s << " := ";
        case _if: return s << "if ";
        case _then: return s << " then ";
        case _else: return s << " else ";
        case _endif: return s << " endif";
        case _ite: throw exception::strange ("<< (token _ite)");
        default: throw  exception::strange ("<< (unknown token)");
        }
    }
  }
}

#endif
