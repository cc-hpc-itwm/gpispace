// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <logging/test/message.hpp>

#include <ostream>
#include <tuple>

namespace fhg
{
  namespace logging
  {
    bool operator== (message const& lhs, message const& rhs)
    {
      return std::tie (lhs._content, lhs._category)
        == std::tie (rhs._content, rhs._category);
    }
    std::ostream& operator<< (std::ostream& os, message const& x)
    {
      return os << "content=" << x._content << ", "
                << "category=" << x._category;
    }
  }
}
