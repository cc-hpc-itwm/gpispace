// Copyright (C) 2010,2012-2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/rapidxml/types.hpp>
#include <gspc/xml/parse/state.fwd.hpp>


  namespace gspc::xml::parse
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
