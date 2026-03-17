// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/boost/program_options/validators/is_directory_if_exists.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<gspc::util::boost::program_options::is_directory_if_exists>
      : ostream_formatter
  {};
}
