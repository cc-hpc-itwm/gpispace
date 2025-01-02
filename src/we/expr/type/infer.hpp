// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/expr/parse/node.hpp>
#include <we/expr/type/Context.hpp>
#include <we/expr/type/Type.hpp>

namespace expr
{
  namespace type
  {
    Type infer (Context&, parse::node::type const&);
  }
}
