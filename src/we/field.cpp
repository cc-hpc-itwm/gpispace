// Copyright (C) 2013,2020-2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/field.hpp>
#include <gspc/we/type/value/peek.hpp>

namespace gspc::pnet
{
  type::value::value_type const& field
    ( std::string const& f
    , type::value::value_type const& v
    , type::signature::signature_type const& signature
    )
  {
    std::optional<std::reference_wrapper<type::value::value_type const>> field
      (type::value::peek (f, v));

    if (!field)
    {
      throw exception::missing_field ( signature
                                     , v
                                     , std::list<std::string> (1, f)
                                     );
    }

    return field->get();
  }
}
