// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/dllexport.hpp>

#include <boost/filesystem/path.hpp>

namespace iml
{
  namespace detail
  {
    class FHG_UTIL_DLLEXPORT Installation
    {
    public:
      Installation();

      ::boost::filesystem::path const server_binary;
      ::boost::filesystem::path const rifd_binary;
    };
  }
}
