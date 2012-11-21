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
    , _tolong, _todouble
    , _bitset_insert, _bitset_delete, _bitset_is_element
    , _bitset_or, _bitset_and, _bitset_xor
    , _bitset_count
    , _bitset_tohex, _bitset_fromhex
    , _stack_empty, _stack_top, _stack_push, _stack_pop, _stack_size
    , _stack_join
    , _map_assign, _map_unassign, _map_is_assigned, _map_get_assignment
    , _map_size, _map_empty
    , _set_insert, _set_erase, _set_is_element
    , _set_pop, _set_top, _set_empty, _set_size
    , _len
    , _substr

    , sep                     // comma
    , lpr, rpr                // parenthesis

    , val                     // value
    , ref                     // reference to context

    , define                  // prec -99, right associative

    , _if, _then, _else, _endif, _ite

    , eof
    };

    static std::ostream & operator << (std::ostream & s, const type & token)
    {
      switch (token)
        {
        case _or: return s << " || ";
        case _and: return s << " && ";
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
        case _tolong: return s << "long";
        case _todouble: return s << "double";
        case _bitset_insert: return s << "bitset_insert";
        case _bitset_delete: return s << "bitset_delete";
        case _bitset_is_element: return s << "bitset_is_element";
        case _bitset_or: return s << "bitset_or";
        case _bitset_and: return s << "bitset_and";
        case _bitset_xor: return s << "bitset_xor";
        case _bitset_count: return s << "bitset_count";
        case _bitset_tohex: return s << "bitset_tohex";
        case _bitset_fromhex: return s << "bitset_fromhex";
        case _stack_empty: return s << "stack_empty";
        case _stack_top: return s << "stack_top";
        case _stack_push: return s << "stack_push";
        case _stack_pop: return s << "stack_pop";
        case _stack_size: return s << "stack_size";
        case _stack_join: return s << "stack_join";
        case _map_assign: return s << "map_assign";
        case _map_unassign: return s << "map_unassign";
        case _map_is_assigned: return s << "map_is_assigned";
        case _map_get_assignment: return s << "map_get_assignement";
        case _map_size: return s << "map_size";
        case _map_empty: return s << "map_empty";
        case _set_insert: return s << "set_insert";
        case _set_erase: return s << "set_erase";
        case _set_is_element: return s << "set_is_element";
        case _set_pop: return s << "set_pop";
        case _set_top: return s << "set_top";
        case _set_empty: return s << "set_empty";
        case _set_size: return s << "set_size";
        case _len: return s << "len";
        case _substr: return s << "substr";
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
