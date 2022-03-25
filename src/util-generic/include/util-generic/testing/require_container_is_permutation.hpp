// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

//! Ensure that the containers \a lhs_ and \a rhs_ are of same size
//! and their contents are a permutation of each other.
//! \note Does not validate recursively, i.e. `vec[a,b] == vec[b,a]`,
//! but `vec[vec[a,b]] != vec[vec[b,a]]`.
#define FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION(lhs_, rhs_) \
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION_IMPL(lhs_, rhs_)

#include <util-generic/testing/require_container_is_permutation.ipp>
