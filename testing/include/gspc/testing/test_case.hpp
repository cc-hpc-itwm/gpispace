// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//! Add a test case name_ for every template_type_ in types_.
#define GSPC_TESTING_TEMPLATED_CASE_T(name_, template_type_, types_...)    \
  GSPC_TESTING_TEMPLATED_CASE_IMPL (name_, template_type_, types_)

//! Add a test case name_ for every T in types_.
//! \note Convenience overload of GSPC_TESTING_TEMPLATED_CASE_T
//! with template_type_ being T.
#define GSPC_TESTING_TEMPLATED_CASE(name_, types_...)                      \
  GSPC_TESTING_TEMPLATED_CASE_T (name_, T, types_)

#include <gspc/testing/test_case.ipp>
