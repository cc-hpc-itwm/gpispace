// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    std::string make_backtrace (std::string const& reason);

    class backtracing_exception : public std::runtime_error
    {
    public:
      backtracing_exception (std::string const& reason);
    };
  }
}
