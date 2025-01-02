// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <tuple>
#include <utility>

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
            (Callable&& f, Tuple&& t, std::index_sequence<I...>)
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
        [[deprecated ("use std::apply instead, will be removed after 2025/12/31")]]
        constexpr auto apply (Callable&& f, Tuple&& t)
        -> decltype (detail::apply_impl
                       ( std::forward<Callable> (f)
                       , std::forward<Tuple> (t)
                       , std::make_index_sequence
                           <std::tuple_size<detail::decay_t<Tuple>>{}>{}
                       )
                    )
      {
        return detail::apply_impl
          ( std::forward<Callable> (f)
          , std::forward<Tuple> (t)
          , std::make_index_sequence
              <std::tuple_size<detail::decay_t<Tuple>>{}>{}
          );
      }
    }
  }
}
