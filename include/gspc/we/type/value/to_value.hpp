// Copyright (C) 2013,2015,2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/value.hpp>



    namespace gspc::pnet::type::value
    {
      template<typename T>
        inline value_type to_value (T const& x)
      {
        return x;
      }
    }
