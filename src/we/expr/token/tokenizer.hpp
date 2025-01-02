// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/token/type.hpp>

#include <we/type/value.hpp>

#include <fhg/util/parse/position.hpp>

#include <list>
#include <string>

namespace expr
{
  namespace token
  {
    struct tokenizer
    {
    public:
      tokenizer (fhg::util::parse::position&);
      pnet::type::value::value_type const& value() const;
      token::type const& token() const;
      void operator++();
      std::list<std::string> const& get_ref() const;

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
      bool is_eof();
      fhg::util::parse::position& pos();

    private:
      fhg::util::parse::position& _pos;
      token::type _token {eof};
      pnet::type::value::value_type _tokval{};
      std::list<std::string> _ref{};

      void skip_comment (std::size_t);
    };
  }
}
