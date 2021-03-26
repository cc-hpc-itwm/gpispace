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
