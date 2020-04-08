#pragma once

#include <we/type/id.hpp>
#include <we/type/value.hpp>

#include <utility>
#include <vector>

namespace we
{
    namespace type
    {
      class activity_t;

      using TokenOnPort = std::pair< pnet::type::value::value_type
                                   , we::port_id_type
                                   >;
      using TokensOnPorts = std::vector<TokenOnPort>;
    }
}
