// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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
