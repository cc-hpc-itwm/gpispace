// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace fhg
{
  namespace util
  {
    namespace mp
    {
      namespace detail
      {
        template<template<typename...> class To, typename> struct rebind;
      }

      //! Rebind a variadic template `T = Sequence<U...>` to `To<U...>`.
      template<template<typename...> class To, typename T>
        using rebind = typename detail::rebind<To, T>::type;
    }
  }
}

#include <util-generic/mp/rebind.ipp>
