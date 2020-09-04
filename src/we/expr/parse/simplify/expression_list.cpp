// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <we/expr/parse/simplify/expression_list.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      expression_list::expression_list (const parser& p)
        : _nodes (p.begin(), p.end())
      { }

      const expression_list::nodes_type& expression_list::nodes() const
      {
        return _nodes;
      }

      expression_list::nodes_type::iterator expression_list::begin()
      {
        return _nodes.begin();
      }
      expression_list::nodes_type::iterator expression_list::end()
      {
        return _nodes.end();
      }

      expression_list::nodes_type::reverse_iterator expression_list::rbegin()
      {
        return _nodes.rbegin();
      }
      expression_list::nodes_type::reverse_iterator expression_list::rend()
      {
        return _nodes.rend();
      }

      expression_list::nodes_type::const_iterator expression_list::begin() const
      {
        return _nodes.begin();
      }
      expression_list::nodes_type::const_iterator expression_list::end() const
      {
        return _nodes.end();
      }

      expression_list::nodes_type::iterator expression_list::erase
        (const nodes_type::iterator& it)
      {
        return _nodes.erase (it);
      }
      void expression_list::insert
        (const nodes_type::iterator& it, nodes_type::value_type& value)
      {
        _nodes.insert (it, value);
      }
    }
  }
}
