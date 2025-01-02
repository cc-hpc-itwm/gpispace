// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//! Require that the `{};` scope started after this function executes
//! in a maximum of \a timeout_.
//!
//! Example usage:
//!
//!   FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (1))
//!   {
//!     std::this_thread::sleep_for (std::chrono::milliseconds (999));
//!   };
//!
//! \note Scope requires to be a `{}` block or convertible to a
//! `void()` function.
#define FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME(timeout_) \
  FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME_IMPL (timeout_)

#include <util-generic/testing/require_maximum_running_time.ipp>
