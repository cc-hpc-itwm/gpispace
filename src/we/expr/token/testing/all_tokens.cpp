// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <we/expr/token/testing/all_tokens.hpp>

namespace expr
{
  namespace token
  {
    namespace testing
    {
      std::list<type> const& all_tokens()
      {
        static std::list<type> const _
          { _or_boolean
          , _or_integral
          , _and_boolean
          , _and_integral
          , _not
          , lt
          , le
          , gt
          , ge
          , ne
          , eq
          , add
          , sub
          , mul
          , div
          , divint
          , modint
          , _pow
          , _powint
          , neg
          , min
          , max
          , abs
          , _floor
          , _ceil
          , _round
          , _sin
          , _cos
          , _sqrt
          , _log
          , _toint
          , _tolong
          , _touint
          , _toulong
          , _tofloat
          , _todouble
          , _bitset_insert
          , _bitset_delete
          , _bitset_is_element
          , _bitset_or
          , _bitset_and
          , _bitset_xor
          , _bitset_count
          , _bitset_tohex
          , _bitset_fromhex
          , _stack_empty
          , _stack_top
          , _stack_push
          , _stack_pop
          , _stack_size
          , _stack_join
          , _map_assign
          , _map_unassign
          , _map_is_assigned
          , _map_get_assignment
          , _map_size
          , _map_empty
          , _set_insert
          , _set_erase
          , _set_is_element
          , _set_pop
          , _set_top
          , _set_empty
          , _set_size
          , _set_is_subset
          , sep
          , lpr
          , rpr
          , val
          , ref
          , define
          , eof
          };

        return _;
      }
    }
  }
}
