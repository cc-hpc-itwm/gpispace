// Copyright (C) 2019-2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <filesystem>
#include <optional>
#include <gspc/util/serialization/std/filesystem/path.hpp>
#include <gspc/util/serialization/std/optional.hpp>

namespace gspc
{
  struct GSPC_EXPORT Certificates
  {
    Certificates() = default;
    Certificates (std::filesystem::path p) : path {p} {}

    std::optional<std::filesystem::path> path {std::nullopt};

    template<typename Archive>
      void serialize (Archive& ar, unsigned int)
    {
      ar & path;
    }
  };
}
