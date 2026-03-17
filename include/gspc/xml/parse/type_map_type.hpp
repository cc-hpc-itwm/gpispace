// Copyright (C) 2012-2015,2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>



    namespace gspc::xml::parse::type
    {
      using type_map_type = std::unordered_map<std::string, std::string>;
      using type_get_type = std::unordered_set<std::string>;
    }
