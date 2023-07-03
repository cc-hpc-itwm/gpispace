// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/token/tokenizer.hpp>
#include <we/expr/token/type.hpp>

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

      using type = ::boost::variant< pnet::type::value::value_type
                                   , Key
                                   , ::boost::recursive_wrapper<unary_t>
                                   , ::boost::recursive_wrapper<binary_t>
                                   , ::boost::recursive_wrapper<ternary_t>
                                   >;

      std::ostream & operator << (std::ostream &, type const&);
      pnet::type::value::value_type const& get (type const&);
      bool is_value (type const&);
      bool is_ref (type const&);
      void rename (type&, std::string const& from, std::string const& to);
      void collect_key_roots (type const&, KeyRoots&);

      struct unary_t
      {
        token::type token;
        type child;

        unary_t (token::type const& token, type const& child);
      };

      struct binary_t
      {
        token::type token;
        type l;
        type r;

        binary_t (token::type const& token, type const& l, type const& r);
      };

      struct ternary_t
      {
        token::type token;
        type child0;
        type child1;
        type child2;

        ternary_t ( token::type const& token
                  , type const& child0
                  , type const& child1
                  , type const& child2
                  );
      };
    }
  }
}
