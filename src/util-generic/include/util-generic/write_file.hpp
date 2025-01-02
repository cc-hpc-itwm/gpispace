// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/detail/dllexport.hpp>
#include <boost/filesystem.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace error::write_file
    {
      auto UTIL_GENERIC_DLLEXPORT could_not_open
        ( std::filesystem::path const&
        ) -> std::runtime_error
        ;
      auto UTIL_GENERIC_DLLEXPORT could_not_write
        ( std::filesystem::path const&
        ) -> std::runtime_error
        ;
    }

    template<typename T>
      void write_file (std::filesystem::path const& path, T const& x)
    {
      auto stream {std::ofstream {path}};

      if (!stream)
      {
        throw error::write_file::could_not_open (path);
      }

      stream << x;

      if (!stream)
      {
        throw error::write_file::could_not_write (path);
      }
    }

    template<typename T>
      [[deprecated ("use write_file (std::filesystem::path")]]
      void write_file (::boost::filesystem::path const& path, T const& x)
    {
      return write_file (std::filesystem::path {path.string()}, x);
    }
  }
}
