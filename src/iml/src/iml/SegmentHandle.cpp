// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/SegmentHandle.hpp>

#include <boost/lexical_cast.hpp>

namespace iml
{
  SegmentHandle::SegmentHandle (std::uint64_t raw_handle)
    : handle (raw_handle)
  {}

  SegmentHandle::SegmentHandle (std::string const& serialized)
  {
    *this = ::boost::lexical_cast<SegmentHandle> (serialized);
  }
  std::string SegmentHandle::to_string() const
  {
    return ::boost::lexical_cast<std::string> (*this);
  }

  bool operator== (SegmentHandle const& lhs, SegmentHandle const& rhs)
  {
    return lhs.handle == rhs.handle;
  }
  bool operator< (SegmentHandle const& lhs, SegmentHandle const& rhs)
  {
    return lhs.handle < rhs.handle;
  }

  std::ostream& operator<< (std::ostream& os, SegmentHandle const& h)
  {
    std::ios_base::fmtflags const saved_flags (os.flags());

    os << "0x";
    os.flags (std::ios::hex);
    os << h.handle;

    os.flags (saved_flags);

    return os;
  }

  std::istream& operator>> (std::istream& is, SegmentHandle& h)
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
  size_t hash<iml::SegmentHandle>::operator()
    (iml::SegmentHandle const& x) const
  {
    return std::hash<std::uint64_t>() (x.handle);
  }
}
