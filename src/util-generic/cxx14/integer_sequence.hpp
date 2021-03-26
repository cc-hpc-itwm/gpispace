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

#if HAS_STD_INTEGER_SEQUENCE

#include <utility>

namespace fhg
{
  namespace util
  {
    namespace cxx14
    {
      using std::index_sequence;
      using std::index_sequence_for;
      using std::integer_sequence;
      using std::make_index_sequence;
      using std::make_integer_sequence;
    }
  }
}

#else

#include <cstddef>
#include <type_traits>

//! \note n3658: Compile-time integer sequences

namespace fhg
{
  namespace util
  {
    namespace cxx14
    {
      template<typename T, T... I>
        struct integer_sequence
      {
        static_assert (std::is_integral<T>::value, "requires integral");

        using value_type = T;
        static constexpr const T size() { return sizeof... (I); }

        //! \note Not standard C++14: Not adopted from the original paper!
        using _next = integer_sequence<T, I..., size()>;
      };

      namespace impl
      {
        template<typename T, std::size_t N>
          struct iota
        {
          using type = typename iota<T, N - 1>::type::_next;
        };

        template<typename T>
          struct iota<T, 0ul>
        {
          using type = integer_sequence<T>;
        };

        template<typename T, T N>
          struct make_integer_sequence
        {
          static_assert
            (!std::is_signed<T>{} || N >= T {0}, "N cannot be negative");

          using type = typename impl::iota<T, (N >= T {0}) ? N : 0>::type;
        };
      }

      template<typename T, T N>
        using make_integer_sequence
          = typename impl::make_integer_sequence<T, N>::type;

      template<std::size_t... I>
        using index_sequence = integer_sequence<std::size_t, I...>;

      template<std::size_t N>
        using make_index_sequence = make_integer_sequence<std::size_t, N>;

      namespace impl
      {
        //! \note workaround for clang bug 22942
        //! https://llvm.org/bugs/show_bug.cgi?id=22942
        template<typename... T>
          struct index_sequence_for_t
        {
          using type = make_index_sequence<sizeof... (T)>;
        };
      }
      template<typename... T>
        using index_sequence_for
          = typename impl::index_sequence_for_t<T...>::type;
    }
  }
}

#endif
