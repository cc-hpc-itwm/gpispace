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

#include <iml/AllocationHandle.hpp>

#include <boost/lexical_cast.hpp>

namespace iml
{
  AllocationHandle::AllocationHandle (std::uint64_t raw_handle)
    : handle (raw_handle)
  {}

  AllocationHandle::AllocationHandle (std::string const& serialized)
  {
    *this = boost::lexical_cast<AllocationHandle> (serialized);
  }
  std::string AllocationHandle::to_string() const
  {
    return boost::lexical_cast<std::string> (*this);
  }

  bool operator== (AllocationHandle const& lhs, AllocationHandle const& rhs)
  {
    return lhs.handle == rhs.handle;
  }
  bool operator< (AllocationHandle const& lhs, AllocationHandle const& rhs)
  {
    return lhs.handle < rhs.handle;
  }

  std::ostream& operator<< (std::ostream& os, AllocationHandle const& h)
  {
    std::ios_base::fmtflags const saved_flags (os.flags());

    os << "0x";
    os.flags (std::ios::hex);
    os << h.handle;

    os.flags (saved_flags);

    return os;
  }

  std::istream& operator>> (std::istream& is, AllocationHandle& h)
  {
    std::ios_base::fmtflags const saved_flags (is.flags());
    is.flags (std::ios::hex);
    is >> h.handle;

    is.flags (saved_flags);
    return is;
  }
}

namespace std
{
  size_t hash<iml::AllocationHandle>::operator()
    (iml::AllocationHandle const& x) const
  {
    return std::hash<std::uint64_t>() (x.handle);
  }
}
