// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/token/type.hpp>

#include <boost/format.hpp>

#include <iostream>
#include <stdexcept>

namespace expr
{
  namespace token
  {
    show::show (const type& token)
      : _token (token)
    {}
    std::ostream& show::operator() (std::ostream& os) const
    {
      switch (_token)
        {
        case _or: return os << " || ";
        case _and: return os << " && ";
        case _not: return os << "!";
        case lt: return os << " < ";
        case le: return os << " <= ";
        case gt: return os << " > ";
        case ge: return os << " >= ";
        case ne: return os << " != ";
        case eq: return os << " == ";
        case add: return os << " + ";
        case sub: return os << " - ";
        case mul: return os << " * ";
        case div: return os << " / ";
        case divint: return os << " div ";
        case mod: return os << " % ";
        case modint: return os << " mod ";
        case _pow: return os << "**";
        case _powint: return os << "^";
        case neg: return os << "-";
        case min: return os << "min";
        case max: return os << "max";
        case _floor: return os << "floor";
        case _ceil: return os << "ceil";
        case _round: return os << "round";
        case _sin: return os << "sin";
        case _cos: return os << "cos";
        case _sqrt: return os << "sqrt";
        case _log: return os << "log";
        case _toint: return os << "int";
        case _tolong: return os << "long";
        case _touint: return os << "uint";
        case _toulong: return os << "ulong";
        case _tofloat: return os << "float";
        case _todouble: return os << "double";
        case _bitset_insert: return os << "bitset_insert";
        case _bitset_delete: return os << "bitset_delete";
        case _bitset_is_element: return os << "bitset_is_element";
        case _bitset_or: return os << "bitset_or";
        case _bitset_and: return os << "bitset_and";
        case _bitset_xor: return os << "bitset_xor";
        case _bitset_count: return os << "bitset_count";
        case _bitset_tohex: return os << "bitset_tohex";
        case _bitset_fromhex: return os << "bitset_fromhex";
        case _stack_empty: return os << "stack_empty";
        case _stack_top: return os << "stack_top";
        case _stack_push: return os << "stack_push";
        case _stack_pop: return os << "stack_pop";
        case _stack_size: return os << "stack_size";
        case _stack_join: return os << "stack_join";
        case _map_assign: return os << "map_assign";
        case _map_unassign: return os << "map_unassign";
        case _map_is_assigned: return os << "map_is_assigned";
        case _map_get_assignment: return os << "map_get_assignment";
        case _map_size: return os << "map_size";
        case _map_empty: return os << "map_empty";
        case _set_insert: return os << "set_insert";
        case _set_erase: return os << "set_erase";
        case _set_is_element: return os << "set_is_element";
        case _set_pop: return os << "set_pop";
        case _set_top: return os << "set_top";
        case _set_empty: return os << "set_empty";
        case _set_size: return os << "set_size";
        case _set_is_subset: return os << "set_is_subset";
        case _len: return os << "len";
        case _substr: return os << "substr";
        case abs: return os << "abs";
        case sep: return os << ", ";
        case lpr: return os << "(";
        case rpr: return os << ")";
        case val: return os << "<val>";
        case ref: return os << "<ref>";
        case eof: return os << "<eof>";
        case define: return os << " := ";
        default: throw std::runtime_error
            (( boost::format ("token::show (%1%)")
             % static_cast<int> (_token)
             ).str()
            );
        }
    }
    std::ostream& operator<< (std::ostream& os, const show& s)
    {
      return s (os);
    }
    std::ostream& operator<< (std::ostream& os, const type& x)
    {
      return os << show (x);
    }
  }
}
