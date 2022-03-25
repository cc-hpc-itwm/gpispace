// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
