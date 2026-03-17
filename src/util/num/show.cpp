// Copyright (C) 2013,2015,2020-2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/num/show.hpp>

#include <boost/multiprecision/cpp_int.hpp>

#include <iostream>

namespace
{
  class visitor_show
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
    std::ostream& operator() (boost::multiprecision::cpp_int const& x) const
    {
      return _os << x << "A";
    }

  private:
    std::ostream& _os;
  };
}

std::ostream& operator<< (std::ostream& os, gspc::util::num_type const& v)
{
  const std::ios_base::fmtflags ff (os.flags());
  os << std::showpoint;
  std::visit (visitor_show (os), v);
  os.flags (ff);
  return os;
}
