// Copyright (C) 2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>

namespace gspc::testing
{
  [[nodiscard]] auto unique_path() -> std::filesystem::path;
}
