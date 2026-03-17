// Copyright (C) 2023-2024 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/blank.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<::boost::blank> : ostream_formatter
  {};
}
