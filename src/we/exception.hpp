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

#include <we/type/value.hpp>
#include <we/type/signature.hpp>

#include <we/expr/token/type.hpp>

#include <list>
#include <stdexcept>

namespace pnet
{
  namespace exception
  {
#define MEMBER(_name, _type...)                           \
    public:                                               \
      const _type& _name() const { return _ ## _name; }   \
    private:                                              \
      _type _ ## _name

    class type_error : public std::runtime_error
    {
    public:
      type_error (const std::string&);
      ~type_error() throw() {}
    };

    class type_mismatch : public type_error
    {
    public:
      type_mismatch ( const type::signature::signature_type&
                    , const type::value::value_type&
                    , const std::list<std::string>&
                    );
      ~type_mismatch() throw() {}

      MEMBER (signature, type::signature::signature_type);
      MEMBER (value, type::value::value_type);
      MEMBER (path, std::list<std::string>);
    };

    class missing_field : public type_error
    {
    public:
      missing_field ( const type::signature::signature_type&
                    , const type::value::value_type&
                    , const std::list<std::string>&
                    );
      ~missing_field() throw() {}

      MEMBER (signature, type::signature::signature_type);
      MEMBER (value, type::value::value_type);
      MEMBER (path, std::list<std::string>);
    };

    class unknown_field : public type_error
    {
    public:
      unknown_field ( const type::value::value_type&
                    , const std::list<std::string>&
                    );
      ~unknown_field() throw() {}

      MEMBER (value, type::value::value_type);
      MEMBER (path, std::list<std::string>);
    };

    class eval : public type_error
    {
    public:
      eval (const ::expr::token::type&, const type::value::value_type&);
      eval ( const ::expr::token::type&
           , const type::value::value_type&
           , const type::value::value_type&
           );
      ~eval() throw() {}

      MEMBER (token, ::expr::token::type);
      MEMBER (values, std::list<type::value::value_type>);
    };

    class missing_binding : public std::runtime_error
    {
    public:
      missing_binding (const std::string&);
      ~missing_binding() throw() {}

      MEMBER (key, std::string);
    };

    class could_not_resolve : public std::runtime_error
    {
    public:
      could_not_resolve (const std::string&, const std::list<std::string>&);
      ~could_not_resolve() throw() {}

      MEMBER (type, std::string);
      MEMBER (path, std::list<std::string>);
    };

    namespace port
    {
      class unknown : public std::runtime_error
      {
      public:
        unknown ( const std::string& transition_name
                , const std::string& port_name
                );
        ~unknown() throw() {}

        MEMBER (transition_name, std::string);
        MEMBER (port_name, std::string);
      };
    }

#undef MEMBER
  }
}
