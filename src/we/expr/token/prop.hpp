// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/token/type.hpp>

namespace expr
{
  namespace token
  {
    bool is_builtin (type const&);
    bool is_prefix (type const&);
    bool next_can_be_unary (type const&);
    bool is_define (type const&);
    bool is_or_boolean (type const&);
    bool is_and_boolean (type const&);
  }
}
