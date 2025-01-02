// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/detail/dllexport.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace error::read_file
    {
      auto UTIL_GENERIC_DLLEXPORT could_not_open
        ( std::filesystem::path const&
        ) -> std::runtime_error
        ;
    }

    template<typename T = std::string>
      T read_file (std::filesystem::path const& path)
    {
      std::ifstream ifs (path, std::ifstream::binary);

      if (not ifs)
      {
        throw error::read_file::could_not_open (path);
      }

      ifs >> std::noskipws;

      return {std::istream_iterator<char> (ifs), std::istream_iterator<char>()};
    }

    template<typename T>
      T read_file_as (std::filesystem::path const& path)
    {
      return ::boost::lexical_cast<T> (read_file (path));
    }

    template<typename T = std::string>
      [[deprecated ("use read_file (std::filesystem::path)")]]
      T read_file (::boost::filesystem::path const& path)
    {
      return read_file<T> (std::filesystem::path {path.string()});
    }

    template<typename T>
      [[deprecated ("use read_file_as (std::filesystem::path)")]]
      T read_file_as (::boost::filesystem::path const& path)
    {
      return ::boost::lexical_cast<T> (read_file (path));
    }
  }
}
