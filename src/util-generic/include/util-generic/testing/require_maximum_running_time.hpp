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
