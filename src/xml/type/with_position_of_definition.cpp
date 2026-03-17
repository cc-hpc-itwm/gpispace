// Copyright (C) 2013,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/with_position_of_definition.hpp>


  namespace gspc::xml::parse
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
