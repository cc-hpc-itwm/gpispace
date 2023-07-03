// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/util/position.hpp>

namespace xml
{
  namespace parse
  {
    class with_position_of_definition
    {
    public:
      with_position_of_definition (util::position_type const&);

      util::position_type const& position_of_definition() const;

    protected:
      util::position_type _position_of_definition;
    };
  }
}
