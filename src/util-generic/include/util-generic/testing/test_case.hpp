// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//! Add a test case name_ for every template_type_ in types_.
#define FHG_UTIL_TESTING_TEMPLATED_CASE_T(name_, template_type_, types_...)    \
  FHG_UTIL_TESTING_TEMPLATED_CASE_IMPL (name_, template_type_, types_)

//! Add a test case name_ for every T in types_.
//! \note Convenience overload of FHG_UTIL_TESTING_TEMPLATED_CASE_T
//! with template_type_ being T.
#define FHG_UTIL_TESTING_TEMPLATED_CASE(name_, types_...)                      \
  FHG_UTIL_TESTING_TEMPLATED_CASE_T (name_, T, types_)

#include <util-generic/testing/test_case.ipp>
