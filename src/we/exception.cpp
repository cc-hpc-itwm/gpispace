// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <we/exception.hpp>

#include <we/type/signature/show.hpp>
#include <we/type/value/path/join.hpp>
#include <we/type/value/show.hpp>
#include <we/signature_of.hpp>

#include <boost/format.hpp>

namespace pnet
{
  namespace exception
  {
    type_error::type_error (std::string const& msg)
      : std::runtime_error ((boost::format ("type error: %1%") % msg).str())
    {}
    type_mismatch::type_mismatch
    ( type::signature::signature_type const& signature
    , type::value::value_type const& value
    , std::list<std::string> const& path
    )
      : type_error
        ( ( boost::format ( "type mismatch for field '%2%': expected type '%1%'"
                            ", value '%4%' has type '%3%'"
                          )
          % type::signature::show (signature)
          % type::value::path::join (path)
          % type::signature::show (signature_of (value))
          % type::value::show (value)
          ).str()
        )
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
        ( ( boost::format ("missing field '%2%' of type '%1%' in value '%3%'")
          % type::signature::show (signature)
          % type::value::path::join (path)
          % type::value::show (value)
          ).str()
        )
      , _signature (signature)
      , _value (value)
      , _path (path)
    {}
    unknown_field::unknown_field
    ( type::value::value_type const& value
    , std::list<std::string> const& path
    )
      : type_error
        ( ( boost::format ("unknown field '%1%' with value '%2%' of type '%3%'")
          % type::value::path::join (path)
          % type::value::show (value)
          % type::signature::show (signature_of (value))
          ).str()
        )
      , _value (value)
      , _path (path)
    {}
    eval::eval ( const ::expr::token::type& token
               , type::value::value_type const& x
               )
      : type_error
        ( ( boost::format ("eval %1% (%2%)")
          % token
          % type::value::show (x)
          ).str()
        )
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
        ( ( boost::format ("eval %1% (%2%, %3%)")
          % ::expr::token::show (token)
          % type::value::show (l)
          % type::value::show (r)
          ).str()
        )
      , _token (token)
      , _values()
    {
      _values.push_back (l);
      _values.push_back (r);
    }
    missing_binding::missing_binding (std::string const& key)
      : std::runtime_error
        ((boost::format ("missing binding for: ${%1%}") % key).str())
      , _key (key)
    {}
    could_not_resolve::could_not_resolve ( std::string const& type
                                         , std::list<std::string> const& path
                                         )
      : std::runtime_error
        ((boost::format ("could not resolve type '%1%' for field '%2%'")
         % type
         % type::value::path::join (path)
         ).str()
        )
      , _type (type)
      , _path (path)
    {}

    namespace port
    {
      unknown::unknown ( std::string const& transition_name
                       , std::string const& port_name
                       )
        : std::runtime_error
          ( ( boost::format ("in transiton '%1%': unknown port '%2%'")
            % transition_name
            % port_name
            ).str()
          )
        , _transition_name (transition_name)
        , _port_name (port_name)
      {}
    }
  }
}
