// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <boost/filesystem/path.hpp> // deprecated

#include <filesystem>
#include <optional>
#include <util-generic/serialization/std/filesystem/path.hpp>
#include <util-generic/serialization/std/optional.hpp>

namespace gspc
{
  struct GSPC_DLLEXPORT Certificates
  {
    Certificates() = default;
    Certificates (std::filesystem::path p) : path {p} {}

    [[deprecated ("use Certificates (std::filesystem::path)")]]
    Certificates (::boost::filesystem::path p)
      : path {std::filesystem::path {p.string()}}
    {}

    std::optional<std::filesystem::path> path {std::nullopt};

    template<typename Archive>
      void serialize (Archive& ar, unsigned int)
    {
      ar & path;
    }
  };
}
