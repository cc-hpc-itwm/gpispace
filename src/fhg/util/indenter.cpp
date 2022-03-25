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

#include <fhg/util/indenter.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    indenter::indenter (unsigned int depth)
      : _depth (depth)
    {}
    indenter& indenter::operator++()
    {
      ++_depth;
      return *this;
    }
    indenter indenter::operator++ (int)
    {
      indenter old (*this);
      ++_depth;
      return old;
    }
    indenter& indenter::operator--()
    {
      --_depth;
      return *this;
    }
    indenter indenter::operator-- (int)
    {
      indenter old (*this);
      --_depth;
      return old;
    }
    std::ostream& indenter::operator() (std::ostream& os) const
    {
      return os << std::endl << std::string (_depth << 1u, ' ');
    }

    deeper::deeper (indenter& indenter)
      : _indenter (indenter)
    {}
    std::ostream& deeper::operator() (std::ostream& os) const
    {
      ++_indenter;
      os << _indenter;
      --_indenter;
      return os;
    }
  }
}
