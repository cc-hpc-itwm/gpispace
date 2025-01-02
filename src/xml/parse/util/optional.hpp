// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/rapidxml/types.hpp>

#include <string>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    ::boost::optional<std::string> optional (const xml_node_type*, const Ch*);
  }
}
