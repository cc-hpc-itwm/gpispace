// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

namespace fhg
{
  namespace util
  {
    //! Equivalent to boost::make_optional (cond, value) except that
    //! evaluation of value is deferred, which is often a requirement
    //! when value is dependend on cond and would require `cond ?
    //! boost::optional<T> (value) : boost::optional<T>()` instead.
#define FHG_UTIL_MAKE_OPTIONAL(cond_, how_...)                      \
    FHG_UTIL_MAKE_OPTIONAL_IMPL(cond_, how_)
  }
}

#include <util-generic/make_optional.ipp>
