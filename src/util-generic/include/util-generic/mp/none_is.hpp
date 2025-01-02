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
      //! Given a type Needle, checks that types Haystack do not
      //! contain Needle.
      template<typename Needle, typename... Haystack>
        struct none_is : std::true_type {};
      template<typename T, typename... Tail>
        struct none_is<T, T, Tail...> : std::false_type {};
      template<typename T, typename Head, typename... Tail>
        struct none_is<T, Head, Tail...> : none_is<T, Tail...> {};
    }
  }
}
