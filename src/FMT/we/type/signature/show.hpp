// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/signature/show.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<pnet::type::signature::show> : fmt::ostream_formatter
  {};
}