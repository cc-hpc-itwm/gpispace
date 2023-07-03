// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/field.hpp>
#include <we/type/value/peek.hpp>

namespace pnet
{
  type::value::value_type const& field
    ( std::string const& f
    , type::value::value_type const& v
    , type::signature::signature_type const& signature
    )
  {
    ::boost::optional<type::value::value_type const&> field
      (type::value::peek (f, v));

    if (!field)
    {
      throw exception::missing_field ( signature
                                     , v
                                     , std::list<std::string> (1, f)
                                     );
    }

    return *field;
  }
}
