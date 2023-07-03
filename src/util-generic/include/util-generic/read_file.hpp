// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    template<typename T = std::string>
      T read_file (::boost::filesystem::path const& path)
    {
      std::ifstream ifs (path.string().c_str(), std::ifstream::binary);

      if (not ifs)
      {
        throw std::runtime_error
          ((::boost::format ("could not open %1%") % path).str());
      }

      ifs >> std::noskipws;

      return {std::istream_iterator<char> (ifs), std::istream_iterator<char>()};
    }

    template<typename T>
      T read_file_as (::boost::filesystem::path const& path)
    {
      return ::boost::lexical_cast<T> (read_file (path));
    }
  }
}
