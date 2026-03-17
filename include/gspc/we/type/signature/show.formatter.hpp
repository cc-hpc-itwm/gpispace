// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/signature/show.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<gspc::pnet::type::signature::show> : fmt::ostream_formatter
  {};
}
