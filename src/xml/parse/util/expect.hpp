// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/rapidxml/types.hpp>
#include <xml/parse/state.fwd.hpp>

namespace xml
{
  namespace parse
  {
    void expect_none_or ( xml_node_type*& node
                        , rapidxml::node_type
                        , state::type const&
                        );
    void expect_none_or ( xml_node_type*& node
                        , rapidxml::node_type
                        , rapidxml::node_type
                        , state::type const&
                        );
  }
}
