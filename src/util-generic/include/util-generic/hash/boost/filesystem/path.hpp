// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>

#include <functional>

namespace std
{
  template<>
    struct hash<::boost::filesystem::path>
  {
    size_t operator() (::boost::filesystem::path const& path) const
    {
      return ::boost::filesystem::hash_value (path);
    }
  };
}
