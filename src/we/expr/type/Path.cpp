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

#include <we/expr/type/Path.hpp>

#include <util-generic/print_container.hpp>

#include <ostream>

namespace expr
{
  namespace type
  {
    Path::Path (Particle particle)
      : Path (Particles ({particle}))
    {}

    Path::const_iterator Path::begin() const
    {
      return std::begin (_particles);
    }
    Path::const_iterator Path::end() const
    {
      return std::end (_particles);
    }

    void Path::emplace_back (Particle particle)
    {
      _particles.emplace_back (particle);
    }
    void Path::pop_back()
    {
      _particles.pop_back();
    }

    std::ostream& operator<< (std::ostream& os, Path const& path)
    {
      return os << fhg::util::print_container ("${", ".", "}", path._particles);
    }
  }
}
