// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/mp/none_is.hpp>
#include <util-generic/mp/rebind.hpp>

#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace mp
    {
      namespace detail
      {
        template<typename> struct remove_duplicates;
        template<typename Sequence, typename Known>
          struct remove_duplicates_impl;
        template<typename...> struct sequence {};

        template<typename Head, typename... Tail>
          struct remove_duplicates<sequence<Head, Tail...>>
            : remove_duplicates_impl<sequence<Tail...>, sequence<Head>>
        {};

        template<>
          struct remove_duplicates<sequence<>>
        {
          using type = sequence<>;
        };

        template< typename Head
                , typename... KnownTypes
                >
          struct remove_duplicates_impl< sequence<Head>
                                       , sequence<KnownTypes...>
                                       >
        {
          using type = typename std::conditional
            < none_is<Head, KnownTypes...>::value
            , sequence<KnownTypes..., Head>
            , sequence<KnownTypes...>
            >::type;
        };

        template< typename Head, typename TailHead, typename... Tail
                , typename... KnownTypes
                >
          struct remove_duplicates_impl< sequence<Head, TailHead, Tail...>
                                       , sequence<KnownTypes...>
                                       >
            : remove_duplicates_impl
                < sequence<TailHead, Tail...>
                , typename remove_duplicates_impl<sequence<Head>, sequence<KnownTypes...>>::type
                >
        {};

        template< template<typename...> class Sequence
                , typename... T
                >
          struct remove_duplicates<Sequence<T...>>
        {
          using type
            = typename rebind < Sequence
                              , typename remove_duplicates<sequence<T...>>::type
                              >::type;
        };
      }
    }
  }
}
