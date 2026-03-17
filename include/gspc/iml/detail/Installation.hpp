// Copyright (C) 2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <filesystem>


  namespace gspc::iml::detail
  {
    class GSPC_EXPORT Installation
    {
    public:
      Installation();

      std::filesystem::path const installation_prefix;
      std::filesystem::path const server_binary;
    };
  }
