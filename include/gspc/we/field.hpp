// Copyright (C) 2013,2015,2021,2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/exception.hpp>
#include <gspc/we/type/signature.hpp>
#include <gspc/we/type/value.hpp>

#include <list>
#include <string>

namespace gspc::pnet
{
  GSPC_EXPORT type::value::value_type const& field
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
