// Copyright (C) 2013-2015,2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/type/value.hpp>

#include <string>



    namespace gspc::util::parse
    {
      class position;
    }





    namespace gspc::pnet::type::value
    {
      GSPC_EXPORT value_type read (util::parse::position&);
      GSPC_EXPORT value_type read (std::string const&);
    }
