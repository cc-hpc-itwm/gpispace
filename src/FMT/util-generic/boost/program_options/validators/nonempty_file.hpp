// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/boost/program_options/validators/nonempty_file.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<fhg::util::boost::program_options::nonempty_file>
      : ostream_formatter
  {};
}
