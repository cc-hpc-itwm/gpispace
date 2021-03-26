// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
