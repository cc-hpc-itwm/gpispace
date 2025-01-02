// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
