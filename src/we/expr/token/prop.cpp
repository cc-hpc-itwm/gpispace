// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <we/expr/token/prop.hpp>

namespace expr
{
  namespace token
  {
    bool is_builtin (type const& token)
    {
      return (token > neg && token < sep);
    }

    bool is_prefix (type const& token)
    {
      switch (token)
        {
        case min:
        case max:
        case _floor:
        case _ceil:
        case _round:
        case _sin:
        case _cos:
        case _bitset_insert:
        case _bitset_delete:
        case _bitset_is_element:
        case _bitset_tohex:
        case _bitset_fromhex:
        case _bitset_or:
        case _bitset_and:
        case _bitset_xor:
        case _bitset_count:
        case _stack_top:
        case _stack_push:
        case _stack_pop:
        case _stack_join:
        case _map_assign:
        case _map_unassign:
        case _map_is_assigned:
        case _map_get_assignment:
        case _set_insert:
        case _set_erase:
        case _set_is_element:
        case _set_is_subset:
        case abs: return true;
        default: return false;
        }
    }

    bool next_can_be_unary (type const& token)
    {
      switch (token)
        {
        case val:
        case rpr:
        case ref: return false;
        default: return true;
        }
    }

    bool is_define (type const& token)
    {
      return (token == define);
    }

    bool is_or_boolean (type const& token)
    {
      return (token == _or_boolean);
    }

    bool is_and_boolean (type const& token)
    {
      return (token == _and_boolean);
    }
  }
}
