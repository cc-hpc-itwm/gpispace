// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/token/type.hpp>

namespace expr
{
  namespace associativity
  {
    enum type {left, right};

    type associativity (token::type const&);
  }
}
