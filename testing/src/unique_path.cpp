// Copyright (C) 2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/unique_path.hpp>

#include <gspc/util/syscall.hpp>

#include <fmt/core.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <random>

namespace gspc::testing
{
  auto unique_path() -> std::filesystem::path
  {
    thread_local auto generator
      { std::mt19937_64 {std::invoke (std::random_device{})}
      };
    auto const now
      { std::chrono::steady_clock::now().time_since_epoch().count()
      };

    return fmt::format
      ( "{:04x}-{:04x}-{:04x}-{:04x}-{:x}-{:x}"
      , static_cast<std::uint16_t> (std::invoke (generator))
      , static_cast<std::uint16_t> (std::invoke (generator))
      , static_cast<std::uint16_t> (std::invoke (generator))
      , static_cast<std::uint16_t> (std::invoke (generator))
      , now
      , gspc::util::syscall::getpid()
      );
  }
}
