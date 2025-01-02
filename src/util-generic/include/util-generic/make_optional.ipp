// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/optional.hpp>

namespace fhg
{
  namespace util
  {
#define FHG_UTIL_MAKE_OPTIONAL_IMPL(cond_, how_...)                  \
    fhg::util::detail::make_optional (cond_, [&] { return how_; })

    namespace detail
    {
      template<typename Fun>
        auto make_optional (bool cond, Fun&& fun)
          -> ::boost::optional<decltype (fun())>
      {
        using T = ::boost::optional<decltype (fun())>;
        return cond ? T (fun()) : T();
      }
    }
  }
}
