// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <FMT/gspc/task/ID.hpp>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <gspc/job/ID.hpp>

namespace fmt
{
  template<>
    struct formatter<gspc::job::ID> : ostream_formatter
  {};
}
