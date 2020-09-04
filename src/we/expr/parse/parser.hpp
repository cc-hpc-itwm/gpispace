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

#pragma once

#include <we/expr/exception.hpp>
#include <we/expr/parse/node.hpp>
#include <we/expr/token/type.hpp>

#include <list>
#include <ostream>
#include <stack>
#include <string>

namespace expr
{
  namespace eval
  {
    struct context;
  }

  namespace exception
  {
    namespace parse
    {
      class missing_operand : public exception
      {
      public:
        missing_operand (const std::size_t k, const std::string & what)
          : exception ("missing " + what + " operand", k) {}
        missing_operand (const std::size_t k)
          : exception ("missing operand", k) {}
      };
    }
  }

  namespace parse
  {
    struct parser
    {
    public:
      typedef node::type nd_t;
      typedef std::list<nd_t> nd_stack_t;

      // iterate through the entries
      typedef nd_stack_t::const_iterator nd_const_it_t;
      typedef nd_stack_t::iterator nd_it_t;

    private:
      typedef std::stack<token::type> op_stack_t;
      op_stack_t op_stack;
      nd_stack_t nd_stack;
      nd_stack_t tmp_stack;

      nd_it_t begin() { return nd_stack.begin(); }
      nd_it_t end() { return nd_stack.end(); }

    public:
      nd_const_it_t begin() const { return nd_stack.begin(); }
      nd_const_it_t end() const { return nd_stack.end(); }

    private:
      void unary (const token::type & token, const std::size_t k);
      void binary (const token::type & token, const std::size_t k);
      void ternary (const token::type & token, const std::size_t k);
      void ite (const std::size_t k);
      void reduce (const std::size_t k);
      void parse (const std::string& input);

    public:
      parser (const std::string & input);
      parser (const nd_stack_t & seq);

      // evaluate the whole stack in order, return the last value
      pnet::type::value::value_type eval_all (eval::context& context) const;
      pnet::type::value::value_type eval_all() const;

      bool is_const_true() const;

      void rename (const std::string& from, const std::string& to);

      node::KeyRoots key_roots() const;

      std::string string() const;
    };
  }
}
