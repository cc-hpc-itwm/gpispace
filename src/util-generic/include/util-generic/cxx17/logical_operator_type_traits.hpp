// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//! \note P0013R1: Logical Operator Type Traits (revision 1)

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      template<typename... B> struct conjunction;
      template<typename... B> struct disjunction;
      template<typename B> struct negation;
    }
  }
}

#include <util-generic/cxx17/logical_operator_type_traits.ipp>
