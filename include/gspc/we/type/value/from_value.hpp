// Copyright (C) 2014-2015,2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/value.hpp>



    namespace gspc::pnet::type::value
    {
      template<typename T>
        inline T from_value (value_type const& v)
      {
        return ::boost::get<T> (v);
      }

      template<>
        inline value_type from_value<> (value_type const& v)
      {
        return v;
      }
    }
