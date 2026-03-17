// Copyright (C) 2013,2015-2016,2020-2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/value.hpp>

#include <gspc/we/expr/token/type.hpp>



    namespace gspc::pnet::type::value
    {
      value_type unary (we::expr::token::type const&, value_type const&);
      value_type binary ( we::expr::token::type const&
                        , value_type const&
                        , value_type const&
                        );
    }
