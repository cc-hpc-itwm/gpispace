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

#include <gspc/detail/dllexport.hpp>

#include <we/exception.hpp>
#include <we/type/signature.hpp>
#include <we/type/value.hpp>

#include <list>
#include <string>

namespace pnet
{
  GSPC_DLLEXPORT type::value::value_type const& field
    ( std::string const&
    , type::value::value_type const&
    , type::signature::signature_type const&
    );

  template<typename T>
    T const& field_as ( std::string const& f
                      , type::value::value_type const& v
                      , type::signature::signature_type const& signature
                      )
  {
    type::value::value_type const& value (field (f, v, signature));

    const T* x (::boost::get<T> (&value));

    if (!x)
    {
      throw exception::type_mismatch ( signature
                                     , value
                                     , std::list<std::string> (1, f)
                                     );
    }

    return *x;
  }
}
