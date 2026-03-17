// Copyright (C) 2013-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/expr/token/type.hpp>
#include <gspc/we/type/signature.hpp>
#include <gspc/we/type/value.hpp>

#include <list>
#include <stdexcept>


  namespace gspc::pnet::exception
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

    class GSPC_EXPORT type_error : public std::runtime_error
    {
    public:
      type_error (std::string const&);

      DTOR_COPY_MOVE_ASSIGN (type_error);
    };

    class GSPC_EXPORT type_mismatch : public type_error
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

    class GSPC_EXPORT missing_field : public type_error
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

    class GSPC_EXPORT unknown_field : public type_error
    {
    public:
      unknown_field ( type::value::value_type const&
                    , std::list<std::string> const&
                    );

      DTOR_COPY_MOVE_ASSIGN (unknown_field);

      MEMBER (value, type::value::value_type);
      MEMBER (path, std::list<std::string>);
    };

    class GSPC_EXPORT eval : public type_error
    {
    public:
      eval (const ::gspc::we::expr::token::type&, type::value::value_type const&);
      eval ( const ::gspc::we::expr::token::type&
           , type::value::value_type const&
           , type::value::value_type const&
           );

      DTOR_COPY_MOVE_ASSIGN (eval);

      MEMBER (token, ::gspc::we::expr::token::type);
      MEMBER (values, std::list<type::value::value_type>);
    };

    class GSPC_EXPORT missing_binding : public std::runtime_error
    {
    public:
      missing_binding (std::string const&);

      DTOR_COPY_MOVE_ASSIGN (missing_binding);

      MEMBER (key, std::string);
    };

    class GSPC_EXPORT could_not_resolve : public std::runtime_error
    {
    public:
      could_not_resolve (std::string const&, std::list<std::string> const&);

      DTOR_COPY_MOVE_ASSIGN (could_not_resolve);

      MEMBER (type, std::string);
      MEMBER (path, std::list<std::string>);
    };

    namespace port
    {
      class GSPC_EXPORT unknown : public std::runtime_error
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

    class GSPC_EXPORT generator_place_overflow : public std::runtime_error
    {
    public:
      generator_place_overflow ( std::string const& place_name
                               , std::string const& type_name
                               );

      DTOR_COPY_MOVE_ASSIGN (generator_place_overflow);

      MEMBER (place_name, std::string);
      MEMBER (type_name, std::string);
    };

#undef DTOR_COPY_MOVE_ASSIGN
#undef MEMBER
  }
