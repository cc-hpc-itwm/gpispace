// Copyright (C) 2015,2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/type/value.hpp>

#include <boost/multiprecision/cpp_int/serialize.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/variant.hpp>

#include <string>



    namespace gspc::pnet::type::value
    {
      GSPC_EXPORT std::string to_string (value_type const&);
      GSPC_EXPORT value_type from_string (std::string const&);
    }
