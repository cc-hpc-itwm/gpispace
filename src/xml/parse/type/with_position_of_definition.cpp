// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/with_position_of_definition.hpp>

namespace xml
{
  namespace parse
  {
    with_position_of_definition::with_position_of_definition
      (util::position_type const& position_of_definition)
        : _position_of_definition (position_of_definition)
    {}

    util::position_type const&
    with_position_of_definition::position_of_definition() const
    {
      return _position_of_definition;
    }
  }
}
