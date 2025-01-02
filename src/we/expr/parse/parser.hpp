// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
      using nd_t = node::type;
      using nd_stack_t = std::list<nd_t>;

      // iterate through the entries
      using nd_const_it_t = nd_stack_t::const_iterator;
      using nd_it_t = nd_stack_t::iterator;

    private:
      using op_stack_t = std::stack<token::type>;
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
