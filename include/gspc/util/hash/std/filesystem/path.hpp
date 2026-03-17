// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Not all standard library implementations provide
// std::hash<std::filesystem::path>.
//
// GSPC_HAS_STD_HASH_FILESYSTEM_PATH is set at
// configure time by CompilerFlags.cmake
// via check_cxx_source_compiles.
//
// When the specialization is absent, provide one using
// std::filesystem::hash_value() which every conforming
// C++17 implementation supplies.

#include <cstddef>
#include <filesystem>
#include <functional>

#if !defined (GSPC_HAS_STD_HASH_FILESYSTEM_PATH)

namespace std
{
  template<>
    struct hash<filesystem::path>
  {
    [[nodiscard]] auto operator()
      ( filesystem::path const& path
      ) const noexcept -> size_t
    {
      return filesystem::hash_value (path);
    }
  };
}

#endif
