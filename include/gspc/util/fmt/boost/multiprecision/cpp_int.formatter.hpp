// Copyright (C) 2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/multiprecision/cpp_int.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<boost::multiprecision::cpp_int> : fmt::ostream_formatter{};
}
