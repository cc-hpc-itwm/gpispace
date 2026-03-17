#pragma once


  namespace gspc::util
  {
    //! Equivalent to ::boost::make_optional (cond, value) except that
    //! evaluation of value is deferred, which is often a requirement
    //! when value is dependend on cond and would require `cond ?
    //! std::optional<T> (value) : std::optional<T>()` instead.
#define FHG_UTIL_MAKE_OPTIONAL(cond_, how_...)                      \
    FHG_UTIL_MAKE_OPTIONAL_IMPL(cond_, how_)
  }


#include <gspc/util/make_optional.ipp>
