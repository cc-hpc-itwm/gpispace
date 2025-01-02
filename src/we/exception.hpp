// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <we/expr/token/type.hpp>
#include <we/type/signature.hpp>
#include <we/type/value.hpp>

#include <list>
#include <stdexcept>

namespace pnet
{
  namespace exception
  {
#define MEMBER(_name, _type...)                           \
    public:                                               \
      _type const& _name() const { return _ ## _name; }   \
    private:                                              \
      _type _ ## _name

#define DTOR_COPY_MOVE_ASSIGN(_name)            \
    ~_name() noexcept override = default;       \
    _name (_name const&) = default;             \
    _name (_name&&) = default;                  \
    _name& operator= (_name const&) = delete;   \
    _name& operator= (_name&&) = delete

    class GSPC_DLLEXPORT type_error : public std::runtime_error
    {
    public:
      type_error (std::string const&);

      DTOR_COPY_MOVE_ASSIGN (type_error);
    };

    class GSPC_DLLEXPORT type_mismatch : public type_error
    {
    public:
      type_mismatch ( type::signature::signature_type const&
                    , type::value::value_type const&
                    , std::list<std::string> const&
                    );

      DTOR_COPY_MOVE_ASSIGN (type_mismatch);

      MEMBER (signature, type::signature::signature_type);
      MEMBER (value, type::value::value_type);
      MEMBER (path, std::list<std::string>);
    };

    class GSPC_DLLEXPORT missing_field : public type_error
    {
    public:
      missing_field ( type::signature::signature_type const&
                    , type::value::value_type const&
                    , std::list<std::string> const&
                    );

      DTOR_COPY_MOVE_ASSIGN (missing_field);

      MEMBER (signature, type::signature::signature_type);
      MEMBER (value, type::value::value_type);
      MEMBER (path, std::list<std::string>);
    };

    class GSPC_DLLEXPORT unknown_field : public type_error
    {
    public:
      unknown_field ( type::value::value_type const&
                    , std::list<std::string> const&
                    );

      DTOR_COPY_MOVE_ASSIGN (unknown_field);

      MEMBER (value, type::value::value_type);
      MEMBER (path, std::list<std::string>);
    };

    class GSPC_DLLEXPORT eval : public type_error
    {
    public:
      eval (const ::expr::token::type&, type::value::value_type const&);
      eval ( const ::expr::token::type&
           , type::value::value_type const&
           , type::value::value_type const&
           );

      DTOR_COPY_MOVE_ASSIGN (eval);

      MEMBER (token, ::expr::token::type);
      MEMBER (values, std::list<type::value::value_type>);
    };

    class GSPC_DLLEXPORT missing_binding : public std::runtime_error
    {
    public:
      missing_binding (std::string const&);

      DTOR_COPY_MOVE_ASSIGN (missing_binding);

      MEMBER (key, std::string);
    };

    class GSPC_DLLEXPORT could_not_resolve : public std::runtime_error
    {
    public:
      could_not_resolve (std::string const&, std::list<std::string> const&);

      DTOR_COPY_MOVE_ASSIGN (could_not_resolve);

      MEMBER (type, std::string);
      MEMBER (path, std::list<std::string>);
    };

    namespace port
    {
      class GSPC_DLLEXPORT unknown : public std::runtime_error
      {
      public:
        unknown ( std::string const& transition_name
                , std::string const& port_name
                );

        DTOR_COPY_MOVE_ASSIGN (unknown);

        MEMBER (transition_name, std::string);
        MEMBER (port_name, std::string);
      };
    }

#undef DTOR_COPY_MOVE_ASSIGN
#undef MEMBER
  }
}
