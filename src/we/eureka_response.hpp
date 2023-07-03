// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/eureka.hpp>

#include <functional>

namespace we
{
  using eureka_response_callback
    = std::function<void (type::eureka_ids_type const& ids)>;
}
