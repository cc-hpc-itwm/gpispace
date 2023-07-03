// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/eval/context.hpp>
#include <we/expr/parse/node.hpp>

namespace expr
{
  namespace eval
  {
    pnet::type::value::value_type eval (context&, expr::parse::node::type const&);
  }
}
