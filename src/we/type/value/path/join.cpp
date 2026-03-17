// Copyright (C) 2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/value/path/join.hpp>

#include <gspc/util/join.hpp>




      namespace gspc::pnet::type::value::path
      {
        std::string join (std::list<std::string> const& path)
        {
          return util::join (path, '.').string();
        }
      }
