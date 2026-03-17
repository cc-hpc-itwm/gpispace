// Copyright (C) 2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>

#include <string>
#include <vector>



    namespace gspc::we::test::buffer_alignment
    {
      struct BufferInfo
      {
        std::string name;
        unsigned long size;
        std::optional<unsigned long> alignment;
      };

      std::string create_net_description (std::vector<BufferInfo> const&);
    }
