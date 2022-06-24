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
