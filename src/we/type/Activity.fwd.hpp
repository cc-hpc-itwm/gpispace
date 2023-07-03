// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/id.hpp>
#include <we/type/value.hpp>

#include <utility>
#include <vector>

namespace we
{
    namespace type
    {
      class Activity;

      using TokenOnPort = std::pair< pnet::type::value::value_type
                                   , we::port_id_type
                                   >;
      using TokensOnPorts = std::vector<TokenOnPort>;
    }
}
