#pragma once

#include <we/type/eureka.hpp>

#include <functional>

namespace we
{
  using eureka_response_callback
    = std::function<void (type::eureka_ids_type const& ids)>;
}
