// Copyright (C) 2014-2015,2020,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/value.hpp>

#include <list>



    namespace gspc::pnet::type::value
    {
      std::list<std::pair<std::list<std::string>, value_type>>
        positions (value_type const&);
    }
