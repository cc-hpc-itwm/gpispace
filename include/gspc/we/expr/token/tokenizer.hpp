// Copyright (C) 2010,2012-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/expr/token/type.hpp>

#include <gspc/we/type/value.hpp>

#include <gspc/util/parse/position.hpp>

#include <list>
#include <string>


  namespace gspc::we::expr::token
  {
    struct tokenizer
    {
    public:
      tokenizer (util::parse::position&);
      pnet::type::value::value_type const& value() const;
      token::type const& token() const;
      void operator++();
      std::list<std::string> const& get_ref() const;
      std::string const& get_shared_cleanup_place() const;

    public:
      void set_token (token::type const&);
      void set_value (pnet::type::value::value_type const&);
      void unary (token::type const&, std::string const&);
      void cmp (token::type const&, token::type const&);
      void negsub();
      void mulpow();
      void or_boolean_integral();
      void and_boolean_integral();
      void divcomment();
      void identifier();
      void notne();
      void shared_value();
      bool is_eof();
      util::parse::position& pos();

    private:
      util::parse::position& _pos;
      token::type _token {eof};
      pnet::type::value::value_type _tokval{};
      std::list<std::string> _ref{};
      std::string _shared_cleanup_place{};

      void skip_comment (std::size_t);
    };
  }
