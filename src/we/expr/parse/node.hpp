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

#pragma once

#include <we/expr/token/type.hpp>
#include <we/expr/token/tokenizer.hpp>

#include <we/type/value.hpp>

#include <boost/variant.hpp>

#include <list>
#include <string>
#include <unordered_set>

namespace expr
{
  namespace parse
  {
    namespace node
    {
      struct unary_t;
      struct binary_t;
      struct ternary_t;

      using Key = std::list<std::string>;
      using KeyRoots = std::unordered_set<std::string>;

      typedef boost::variant < pnet::type::value::value_type
                             , Key
                             , boost::recursive_wrapper<unary_t>
                             , boost::recursive_wrapper<binary_t>
                             , boost::recursive_wrapper<ternary_t>
                             > type;

      std::ostream & operator << (std::ostream &, const type&);
      const pnet::type::value::value_type& get (const type&);
      bool is_value (const type&);
      bool is_ref (const type&);
      void rename (type&, const std::string& from, const std::string& to);
      void collect_key_roots (type const&, KeyRoots&);

      struct unary_t
      {
        token::type token;
        type child;

        unary_t (const token::type& token, const type& child);
      };

      struct binary_t
      {
        token::type token;
        type l;
        type r;

        binary_t (const token::type& token, const type& l, const type& r);
      };

      struct ternary_t
      {
        token::type token;
        type child0;
        type child1;
        type child2;

        ternary_t ( const token::type& token
                  , const type& child0
                  , const type& child1
                  , const type& child2
                  );
      };
    }
  }
}
