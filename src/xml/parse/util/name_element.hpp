// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/rapidxml/types.hpp>
#include <xml/parse/state.fwd.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    std::string name_element (xml_node_type*&, state::type const&);
  }
}
