// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/num/show.hpp>

#include <iostream>

namespace
{
  class visitor_show : public ::boost::static_visitor<std::ostream&>
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
  ::boost::apply_visitor (visitor_show (os), v);
  os.flags (ff);
  return os;
}
