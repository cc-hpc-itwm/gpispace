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

#include <fhg/util/parse/error.hpp>

#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      namespace error
      {
        expected::expected (std::string const& what, position const& inp)
          : generic (::boost::format ("expected '%1%'") % what, inp)
        {}
      }
    }
  }
}
