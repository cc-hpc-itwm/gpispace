// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iosfwd>
#include <exception>

namespace util
{
  struct print_exception
  {
    print_exception (std::exception const&, unsigned depth = 0);
    friend std::ostream& operator<< (std::ostream&, print_exception const&);

  private:
    std::exception const& _exception;
    unsigned _depth;
  };
}
