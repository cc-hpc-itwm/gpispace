// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util/print_exception.hpp>

#include <iostream>
#include <string>

namespace util
{
  print_exception::print_exception
    ( std::exception const& exception
    , unsigned depth
    )
      : _exception (exception)
      , _depth (depth)
  {}
  std::ostream& operator<< (std::ostream& os, print_exception const& p)
  {
    auto const space ([] (unsigned d) { return std::string (2 * d , ' '); });

    os << space (p._depth) << p._exception.what();

    try
    {
      std::rethrow_if_nested (p._exception);
    }
    catch (std::exception const& inner)
    {
      os << ":\n" << print_exception (inner, p._depth + 1);
    }
    catch (...)
    {
      os << ":\n" << space (p._depth + 1) << "unknown exception type";
    }

    return os;
  }
}
