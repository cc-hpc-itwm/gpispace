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

#include <we/expr/token/type.hpp>

#include <we/type/value.hpp>

#include <fhg/util/parse/position.hpp>

#include <string>
#include <list>

namespace expr
{
  namespace token
  {
    struct tokenizer
    {
    public:
      tokenizer (fhg::util::parse::position&);
      const pnet::type::value::value_type& value() const;
      const token::type& token() const;
      void operator++();
      const std::list<std::string>& get_ref() const;

    public:
      void set_token (const token::type&);
      void set_value (const pnet::type::value::value_type&);
      void unary (const token::type&, const std::string&);
      void cmp (const token::type&, const token::type&);
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
      token::type _token;
      pnet::type::value::value_type _tokval;
      std::list<std::string> _ref;

      void skip_comment (const std::size_t);
    };
  }
}
