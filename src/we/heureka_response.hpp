#pragma once

#include <we/type/heureka.hpp>

namespace we
{
  using heureka_response_callback
    = std::function<void (type::heureka_ids_type const& ids)>;
}
