// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//! Ensure that the containers \a lhs_ and \a rhs_ are of same size
//! and their contents are a permutation of each other.
//! \note Does not validate recursively, i.e. `vec[a,b] == vec[b,a]`,
//! but `vec[vec[a,b]] != vec[vec[b,a]]`.
#define GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION(lhs_, rhs_) \
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION_IMPL(lhs_, rhs_)

#include <gspc/testing/require_container_is_permutation.ipp>
