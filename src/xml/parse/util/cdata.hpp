// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/rapidxml/types.hpp>
#include <xml/parse/state.hpp>

#include <list>
#include <string>

namespace xml
{
  namespace parse
  {
    std::list<std::string> parse_cdata ( const xml_node_type*
                                       , state::type const&
                                       );
  }
}
