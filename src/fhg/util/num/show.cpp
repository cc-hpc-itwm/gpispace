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

#include <fhg/util/num/show.hpp>

#include <iostream>

namespace
{
  class visitor_show : public boost::static_visitor<std::ostream&>
  {
  public:
    visitor_show (std::ostream& os)
      : _os (os)
    {}

    std::ostream& operator() (int const& x) const
    {
      return _os << x;
    }
    std::ostream& operator() (long const& x) const
    {
      return _os << x << "L";
    }
    std::ostream& operator() (unsigned int const& x) const
    {
      return _os << x << "U";
    }
    std::ostream& operator() (unsigned long const& x) const
    {
      return _os << x << "UL";
    }
    std::ostream& operator() (float const& x) const
    {
      return _os << x << "f";
    }
    std::ostream& operator() (double const& x) const
    {
      return _os << x;
    }

  private:
    std::ostream& _os;
  };
}

std::ostream& operator<< (std::ostream& os, fhg::util::num_type const& v)
{
  const std::ios_base::fmtflags ff (os.flags());
  os << std::showpoint;
  boost::apply_visitor (visitor_show (os), v);
  os.flags (ff);
  return os;
}
