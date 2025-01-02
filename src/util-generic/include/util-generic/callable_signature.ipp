// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <functional>
#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace detail
    {
      template<typename Ret, typename... Args>
        struct callable_signature_t<Ret (Args...)> { using type = Ret (Args...); };
      template<typename Ret, typename... Args>
        struct callable_signature_t<Ret (&)(Args...)> { using type = Ret (Args...); };
      template<typename Ret, typename... Args>
        struct callable_signature_t<Ret (*)(Args...)> { using type = Ret (Args...); };
      template<typename C, typename Ret, typename... Args>
        struct callable_signature_t<Ret (C::*)(Args...)> { using type = Ret (Args...); };
      template<typename C, typename Ret, typename... Args>
        struct callable_signature_t<Ret (C::*)(Args...) const> { using type = Ret (Args...); };
      template<typename C, typename Ret, typename... Args>
        struct callable_signature_t<Ret (C::*)(Args...) volatile> { using type = Ret (Args...); };
      template<typename C, typename Ret, typename... Args>
        struct callable_signature_t<Ret (C::*)(Args...) const volatile> { using type = Ret (Args...); };
      template<typename T>
        struct callable_signature_t
      {
        using type = typename callable_signature_t
          <decltype (&std::remove_reference<T>::type::operator())>::type;
      };

      template<typename Fun>
        struct return_type_t : return_type_t<callable_signature<Fun>> {};

      template<typename Ret, typename... Args>
        struct return_type_t<Ret (Args...)>
      {
        using type = Ret;
      };
    }

    template<typename T, typename ExpectedReturnType, typename... Args>
      struct is_callable<T, ExpectedReturnType (Args...)>
        : std::is_same < decltype (std::declval<T>() (std::declval<Args>()...))
                       , ExpectedReturnType
                       >
    {};
  }
}
