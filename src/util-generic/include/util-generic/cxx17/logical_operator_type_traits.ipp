// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      template<typename...>
        struct conjunction : std::true_type
      {};
      template<typename B1>
        struct conjunction<B1> : B1
      {};
      template<typename B1, typename... Bn>
        struct conjunction<B1, Bn...>
          : std::conditional<!!B1::value, conjunction<Bn...>, B1>::type
      {};

      template<typename...>
        struct disjunction : std::false_type
      {};
      template<typename B1>
        struct disjunction<B1> : B1
      {};
      template<typename B1, typename... Bn>
        struct disjunction<B1, Bn...>
          : std::conditional<!!B1::value, B1, disjunction<Bn...>>::type
      {};

      template<typename B>
        struct negation : std::integral_constant<bool, !B::value>
      {};
    }
  }
}
