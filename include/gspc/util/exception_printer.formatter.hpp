// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/print_exception.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<gspc::util::exception_printer> : ostream_formatter
  {};
  template<>
    struct formatter<gspc::util::current_exception_printer> : ostream_formatter
  {};
}
