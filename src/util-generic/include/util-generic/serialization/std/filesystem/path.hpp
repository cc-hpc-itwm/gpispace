// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/serialization/split_free.hpp>

#include <filesystem>

namespace boost::serialization
{
  template<typename Archive>
    void load (Archive& ar, std::filesystem::path& path, const unsigned int)
  {
    auto name {std::string{}};
    ar >> name;
    path = std::filesystem::path {name};
  }
  template<typename Archive>
    void save (Archive& ar, std::filesystem::path const& path, const unsigned int)
  {
    auto const name {path.string()};
    ar << name;
  }

  template<typename Archive>
    void serialize
      (Archive& ar, std::filesystem::path& path, const unsigned int version)
  {
    ::boost::serialization::split_free (ar, path, version);
  }
}
