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

#include <we/expr/parse/parser.hpp>

#include <boost/noncopyable.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      class expression_list : boost::noncopyable
      {
      public:
        typedef parser::nd_stack_t nodes_type;

      private:
        nodes_type _nodes;

      public:
        explicit expression_list (const parser&);

        const nodes_type& nodes() const;

        nodes_type::iterator begin();
        nodes_type::iterator end();
        nodes_type::reverse_iterator rbegin();
        nodes_type::reverse_iterator rend();
        nodes_type::const_iterator begin() const;
        nodes_type::const_iterator end() const;

        nodes_type::iterator erase (const nodes_type::iterator&);
        void insert (const nodes_type::iterator&, nodes_type::value_type&);
      };
    }
  }
}
