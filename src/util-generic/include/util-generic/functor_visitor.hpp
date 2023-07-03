// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/variant.hpp>

#include <type_traits>
#include <utility>

namespace fhg
{
  namespace util
  {
    namespace detail
    {
      struct Visit{}; // prevent from ctor that accepts plain forwarding ref
    }

    template<typename Ret, typename... Functors>
      struct functor_visitor;

    template<typename Ret, typename Head, typename... Tail>
      struct functor_visitor<Ret, Head, Tail...>
        : std::remove_reference<Head>::type, functor_visitor<Ret, Tail...>
    {
      using std::remove_reference<Head>::type::operator();
      using functor_visitor<Ret, Tail...>::operator();

      template<typename Head_, typename... Tail_>
        functor_visitor (detail::Visit v, Head_&& head, Tail_&&... tail)
          : std::remove_reference<Head>::type (std::forward<Head_> (head))
          , functor_visitor<Ret, Tail...> (v, std::forward<Tail_> (tail)...)
      {}
    };

    template<typename Ret, typename Head>
      struct functor_visitor<Ret, Head> : std::remove_reference<Head>::type
                                        , functor_visitor<Ret>
    {
      using std::remove_reference<Head>::type::operator();

      template<typename Head_>
        functor_visitor (detail::Visit, Head_&& head)
          : std::remove_reference<Head>::type (std::forward<Head_> (head))
      {}
    };

    template<typename Ret>
      struct functor_visitor<Ret> : ::boost::static_visitor<Ret> {};

    template<typename Ret, typename... Functors>
      functor_visitor<Ret, Functors...> make_visitor (Functors&&... lambdas)
    {
      return {detail::Visit{}, std::forward<Functors> (lambdas)...};
    }

    template<typename Ret, typename Variant, typename Visitor>
      Ret visit (Variant&& variant, Visitor&& visitor)
    {
      return ::boost::apply_visitor (std::forward<Visitor> (visitor), variant);
    }
    template<typename Ret, typename Variant, typename... Functors>
      Ret visit (Variant&& variant, Functors&&... lambdas)
    {
      return visit<Ret>
        (variant, make_visitor<Ret> (std::forward<Functors> (lambdas)...));
    }
  }
}
