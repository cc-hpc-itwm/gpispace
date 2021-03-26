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

#if HAS_STD_APPLY

#include <tuple>

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      using std::apply;
    }
  }
}

#else

#include <util-generic/cxx14/integer_sequence.hpp>

#include <type_traits>

//! \note n3915: apply() call a function with arguments from a tuple (V3)

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      namespace detail
      {
        template <typename Callable, typename Tuple, std::size_t... I>
          constexpr auto apply_impl
            (Callable&& f, Tuple&& t, cxx14::index_sequence<I...>)
          -> decltype (f (std::get<I> (std::forward<Tuple> (t))...))
        {
          return std::forward<Callable> (f)
            (std::get<I> (std::forward<Tuple> (t))...);
        }

        template<typename T>
          using decay_t = typename std::decay<T>::type;
      }

      //! \note differences:
      //! * does not work with a generic Callable (doesn't use std::invoke)
      //! \todo cxx14: decltype (auto)
      template <typename Callable, typename Tuple>
        constexpr auto apply (Callable&& f, Tuple&& t)
        -> decltype (detail::apply_impl
                       ( std::forward<Callable> (f)
                       , std::forward<Tuple> (t)
                       , cxx14::make_index_sequence
                           <std::tuple_size<detail::decay_t<Tuple>>{}>{}
                       )
                    )
      {
        return detail::apply_impl
          ( std::forward<Callable> (f)
          , std::forward<Tuple> (t)
          , cxx14::make_index_sequence
              <std::tuple_size<detail::decay_t<Tuple>>{}>{}
          );
      }
    }
  }
}

#endif
