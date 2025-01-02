// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem/path.hpp>

namespace fhg
{
  namespace util
  {
    namespace syscall
    {
      struct directory
      {
        directory (::boost::filesystem::path const&);
        ~directory();

        directory (directory const&) = delete;
        directory& operator= (directory const&) = delete;
        directory (directory&&) = delete;
        directory& operator= (directory&&) = delete;

        int fd() const;
      private:
        /*DIR*/void* _;
      };
    }
  }
}
