// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace fhg
{
  namespace util
  {
    //! Equivalent to ::boost::make_optional (cond, value) except that
    //! evaluation of value is deferred, which is often a requirement
    //! when value is dependend on cond and would require `cond ?
    //! ::boost::optional<T> (value) : ::boost::optional<T>()` instead.
#define FHG_UTIL_MAKE_OPTIONAL(cond_, how_...)                      \
    FHG_UTIL_MAKE_OPTIONAL_IMPL(cond_, how_)
  }
}

#include <util-generic/make_optional.ipp>
