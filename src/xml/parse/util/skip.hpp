// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/rapidxml/types.hpp>

namespace xml
{
  namespace parse
  {
    void skip (xml_node_type*&, rapidxml::node_type);
  }
}
