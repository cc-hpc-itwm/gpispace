// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <fstream>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    template<typename T>
      void write_file (::boost::filesystem::path const& path, T const& x)
    {
      std::ofstream stream (path.string());

      if (!stream)
      {
        throw std::runtime_error
          ((::boost::format ("Could not open %1% for writing.") % path).str());
      }

      stream << x;

      if (!stream)
      {
        throw std::runtime_error
          ((::boost::format ("Could not write to %1%.") % path).str());
      }
    }
  }
}
