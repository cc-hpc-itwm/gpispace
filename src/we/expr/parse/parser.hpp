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

#include <we/expr/exception.hpp>
#include <we/expr/parse/node.hpp>
#include <we/expr/token/type.hpp>
#include <we/expr/type/Type.hpp>

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
  namespace type
  {
    struct Context;
  }

  namespace exception
  {
    namespace parse
    {
      class missing_operand : public exception
      {
      public:
        missing_operand (std::size_t k, std::string const& what)
          : exception ("missing " + what + " operand", k) {}
        missing_operand (std::size_t k)
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
      bool _do_constant_folding = true;

      nd_it_t begin() { return nd_stack.begin(); }
      nd_it_t end() { return nd_stack.end(); }

    public:
      nd_const_it_t begin() const { return nd_stack.begin(); }
      nd_const_it_t end() const { return nd_stack.end(); }

    private:
      void unary (token::type const& token, std::size_t k);
      void binary (token::type const& token, std::size_t k);
      void ternary (token::type const& token, std::size_t k);
      void ite (std::size_t k);
      void reduce (std::size_t k);
      void parse (std::string const& input);

    public:
      struct DisableConstantFolding{};
      parser (DisableConstantFolding, std::string const& input);
      parser (std::string const& input);
      parser (nd_stack_t const& seq);

      // evaluate the whole stack in order, return the last value
      pnet::type::value::value_type eval_all (eval::context& context) const;
      pnet::type::value::value_type eval_all() const;

      // type check the whole stack and return the last expression type
      Type type_check_all (type::Context&) const;
      Type type_check_all() const;

      bool is_const_true() const;

      void rename (std::string const& from, std::string const& to);

      node::KeyRoots key_roots() const;

      std::string string() const;
    };
  }
}
