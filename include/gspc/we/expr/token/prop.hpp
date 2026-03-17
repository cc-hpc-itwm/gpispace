// Copyright (C) 2010,2013-2015,2020-2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/expr/token/type.hpp>


  namespace gspc::we::expr::token
  {
    bool is_builtin (type const&);
    bool is_prefix (type const&);
    bool next_can_be_unary (type const&);
    bool is_define (type const&);
    bool is_or_boolean (type const&);
    bool is_and_boolean (type const&);
  }
