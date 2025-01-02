// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//! \note P0013R1: Logical Operator Type Traits (revision 1)

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      template<typename... B> struct [[deprecated ("use std::conjunction instead, will be removed after 2025/12/31")]] conjunction;
      template<typename... B> struct [[deprecated ("use std::disjunction instead, will be removed after 2025/12/31")]] disjunction;
      template<typename B> struct [[deprecated ("use std::negation instead, will be removed after 2025/12/31")]] negation;
    }
  }
}

#include <util-generic/cxx17/logical_operator_type_traits.ipp>
