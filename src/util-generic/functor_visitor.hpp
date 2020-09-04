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

#include <boost/variant.hpp>

#include <type_traits>
#include <utility>

namespace fhg
{
  namespace util
  {
    template<typename Ret, typename... Functors>
      struct functor_visitor;

    template<typename Ret, typename Head, typename... Tail>
      struct functor_visitor<Ret, Head, Tail...>
        : std::remove_reference<Head>::type, functor_visitor<Ret, Tail...>
    {
      using std::remove_reference<Head>::type::operator();
      using functor_visitor<Ret, Tail...>::operator();

      template<typename Head_, typename... Tail_>
        functor_visitor (Head_&& head, Tail_&&... tail)
          : std::remove_reference<Head>::type (std::forward<Head_> (head))
          , functor_visitor<Ret, Tail...> (std::forward<Tail_> (tail)...)
      {}
    };

    template<typename Ret, typename Head>
      struct functor_visitor<Ret, Head> : std::remove_reference<Head>::type
                                        , functor_visitor<Ret>
    {
      using std::remove_reference<Head>::type::operator();

      template<typename Head_>
        functor_visitor (Head_&& head)
          : std::remove_reference<Head>::type (std::forward<Head_> (head))
      {}
    };

    template<typename Ret>
      struct functor_visitor<Ret> : boost::static_visitor<Ret> {};

    template<typename Ret, typename... Functors>
      functor_visitor<Ret, Functors...> make_visitor (Functors&&... lambdas)
    {
      return {std::forward<Functors> (lambdas)...};
    }

    template<typename Ret, typename Variant, typename Visitor>
      Ret visit (Variant&& variant, Visitor&& visitor)
    {
      return boost::apply_visitor (std::forward<Visitor> (visitor), variant);
    }
    template<typename Ret, typename Variant, typename... Functors>
      Ret visit (Variant&& variant, Functors&&... lambdas)
    {
      return visit<Ret>
        (variant, make_visitor<Ret> (std::forward<Functors> (lambdas)...));
    }
  }
}
