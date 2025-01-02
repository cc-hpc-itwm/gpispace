// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace mp
    {
      namespace detail
      {
        template <typename T, std::size_t N, typename... Tail>
          struct find<T, N, T, Tail...> : std::integral_constant<std::size_t, N> {};
        template <typename T, std::size_t N, typename Head, typename... Tail>
          struct find<T, N, Head, Tail...> : find<T, N + 1, Tail...> {};
      }
    }
  }
}
