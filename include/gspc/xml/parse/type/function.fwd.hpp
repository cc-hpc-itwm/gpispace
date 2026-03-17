// Copyright (C) 2012-2013,2015-2016,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <list>
#include <string>



    namespace gspc::xml::parse::type
    {
      struct conditions_type : public std::list<std::string>
      {
        std::string flatten() const;
      };

      conditions_type operator+ (conditions_type, conditions_type const&);

      struct function_type;
    }
