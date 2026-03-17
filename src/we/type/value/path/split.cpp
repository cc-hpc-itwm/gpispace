// Copyright (C) 2013-2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/value/path/split.hpp>

#include <gspc/util/split.hpp>




      namespace gspc::pnet::type::value::path
      {
        std::list<std::string> split (std::string const& path)
        {
          return util::split<std::string, std::string> (path, '.');
        }
      }
