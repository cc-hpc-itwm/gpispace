// Copyright (C) 2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/expr/parse/node.hpp>
#include <gspc/we/expr/type/Context.hpp>
#include <gspc/we/expr/type/Type.hpp>


  namespace gspc::we::expr::type
  {
    Type infer (Context&, parse::node::type const&);
  }
