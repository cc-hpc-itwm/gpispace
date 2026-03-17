// Copyright (C) 2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/util/position.hpp>


  namespace gspc::xml::parse
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
