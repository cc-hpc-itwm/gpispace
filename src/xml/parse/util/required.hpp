// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/rapidxml/types.hpp>
#include <xml/parse/state.fwd.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    std::string required
      (std::string const&, const xml_node_type*, const Ch*, state::type const&);
  }
}
