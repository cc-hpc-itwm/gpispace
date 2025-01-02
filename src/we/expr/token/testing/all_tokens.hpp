// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/token/type.hpp>

#include <list>

namespace expr
{
  namespace token
  {
    namespace testing
    {
      std::list<type> const& all_tokens();
    }
  }
}
