// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/token/type.hpp>

#include <iosfwd>

namespace expr
{
  namespace parse
  {
    namespace action
    {
      enum type
      { shift
      , reduce
      , accept
      , error1
      , error2
      , error3
      , error4
      };

      std::ostream& operator<< (std::ostream&, type const&);
      type action (token::type const&, token::type const&);
    }
  }
}
