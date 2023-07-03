// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
