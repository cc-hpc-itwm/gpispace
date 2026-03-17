// Copyright (C) 2010,2013,2015,2020-2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/expr/eval/context.hpp>
#include <gspc/we/expr/parse/node.hpp>


  namespace gspc::we::expr::eval
  {
    pnet::type::value::value_type eval (context&, parse::node::type const&);
  }
