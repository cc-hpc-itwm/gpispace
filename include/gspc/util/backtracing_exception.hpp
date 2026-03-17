// Copyright (C) 2012,2014-2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdexcept>
#include <string>


  namespace gspc::util
  {
    std::string make_backtrace (std::string const& reason);

    class backtracing_exception : public std::runtime_error
    {
    public:
      backtracing_exception (std::string const& reason);
    };
  }
