// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/exception.hpp>

#include <we/signature_of.hpp>
#include <we/type/signature/show.hpp>
#include <we/type/value/path/join.hpp>
#include <we/type/value/show.hpp>

#include <FMT/util-generic/join.hpp>
#include <FMT/we/expr/token/show.hpp>
#include <FMT/we/type/signature/show.hpp>
#include <FMT/we/type/value/show.hpp>
#include <fmt/core.h>

namespace pnet
{
  namespace exception
  {
    type_error::type_error (std::string const& msg)
      : std::runtime_error {fmt::format ("type error: {}", msg)}
    {}
    type_mismatch::type_mismatch
    ( type::signature::signature_type const& signature
    , type::value::value_type const& value
    , std::list<std::string> const& path
    )
      : type_error
        { fmt::format ( "type mismatch for field '{1}': expected type '{0}'"
                        ", value '{3}' has type '{2}'"
                      , type::signature::show (signature)
                      , type::value::path::join (path)
                      , type::signature::show (signature_of (value))
                      , type::value::show (value)
                      )
        }
      , _signature (signature)
      , _value (value)
      , _path (path)
    {}
    missing_field::missing_field
    ( type::signature::signature_type const& signature
    , type::value::value_type const& value
    , std::list<std::string> const& path
    )
      : type_error
        { fmt::format ( "missing field '{1}' of type '{0}' in value '{2}'"
                      , type::signature::show (signature)
                      , type::value::path::join (path)
                      , type::value::show (value)
                      )
        }
      , _signature (signature)
      , _value (value)
      , _path (path)
    {}
    unknown_field::unknown_field
    ( type::value::value_type const& value
    , std::list<std::string> const& path
    )
      : type_error
        { fmt::format ( "unknown field '{}' with value '{}' of type '{}'"
                      , type::value::path::join (path)
                      , type::value::show (value)
                      , type::signature::show (signature_of (value))
                      )
        }
      , _value (value)
      , _path (path)
    {}
    eval::eval ( const ::expr::token::type& token
               , type::value::value_type const& x
               )
      : type_error
        { fmt::format ( "eval {} ({})"
                      , token
                      , type::value::show (x)
                      )
        }
      , _token (token)
      , _values()
    {
      _values.push_back (x);
    }
    eval::eval ( const ::expr::token::type& token
               , type::value::value_type const& l
               , type::value::value_type const& r
               )
      : type_error
        { fmt::format ( "eval {} ({}, {})"
                      , ::expr::token::show (token)
                      , type::value::show (l)
                      , type::value::show (r)
                      )
        }
      , _token (token)
      , _values()
    {
      _values.push_back (l);
      _values.push_back (r);
    }
    missing_binding::missing_binding (std::string const& key)
      : std::runtime_error {fmt::format ("missing binding for: ${{{}}}", key)}
      , _key (key)
    {}
    could_not_resolve::could_not_resolve ( std::string const& type
                                         , std::list<std::string> const& path
                                         )
      : std::runtime_error
        { fmt::format ( "could not resolve type '{}' for field '{}'"
                      , type
                      , type::value::path::join (path)
                      )
        }
      , _type (type)
      , _path (path)
    {}

    namespace port
    {
      unknown::unknown ( std::string const& transition_name
                       , std::string const& port_name
                       )
        : std::runtime_error
          { fmt::format ( "in transiton '{}': unknown port '{}'"
                        , transition_name
                        , port_name
                        )
          }
        , _transition_name (transition_name)
        , _port_name (port_name)
      {}
    }
  }
}
