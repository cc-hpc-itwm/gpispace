// Copyright (C) 2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/AllocationHandle.hpp>

#include <boost/lexical_cast.hpp>

namespace gspc::iml
{
  AllocationHandle::AllocationHandle (std::uint64_t raw_handle)
    : handle (raw_handle)
  {}

  AllocationHandle::AllocationHandle (std::string const& serialized)
  {
    *this = ::boost::lexical_cast<AllocationHandle> (serialized);
  }
  std::string AllocationHandle::to_string() const
  {
    return ::boost::lexical_cast<std::string> (*this);
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
  size_t hash<gspc::iml::AllocationHandle>::operator()
    (gspc::iml::AllocationHandle const& x) const
  {
    return std::hash<std::uint64_t>() (x.handle);
  }
}
