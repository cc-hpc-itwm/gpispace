// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/value.hpp>

#include <we/expr/token/type.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      value_type unary (expr::token::type const&, value_type const&);
      value_type binary ( expr::token::type const&
                        , value_type const&
                        , value_type const&
                        );
    }
  }
}
