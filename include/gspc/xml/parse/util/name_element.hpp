// Copyright (C) 2010,2012-2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/rapidxml/types.hpp>
#include <gspc/xml/parse/state.fwd.hpp>

#include <string>


  namespace gspc::xml::parse
  {
    std::string name_element (xml_node_type*&, state::type const&);
  }
