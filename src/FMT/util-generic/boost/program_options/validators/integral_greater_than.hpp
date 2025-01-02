// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/boost/program_options/validators/positive_integral.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>

namespace fmt
{
  template<typename Base, Base that>
    struct formatter<fhg::util::boost::program_options::integral_greater_than<Base, that>>
      : ostream_formatter
  {};
}
