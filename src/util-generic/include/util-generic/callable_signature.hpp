// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace fhg
{
  namespace util
  {
    namespace detail
    {
      template<typename> struct callable_signature_t;
      template<typename> struct return_type_t;
    }

    //! Get signature of callable Callable t, i.e. t() or
    //! Callable::operator().
    template<typename Callable>
      using callable_signature
        = typename detail::callable_signature_t<Callable>::type;

    //! Test if Callable can be invoked with WithSignature, including
    //! implicit conversions of arguments.
    //! \note included in c++17 N4446/P0077R0: "is_callable, the
    //! missing INVOKE related trait"
    template<typename Callable, typename WithSignature>
      struct is_callable;

    //! Get the return type of a Callable.
    template<typename Callable>
      using return_type = typename detail::return_type_t<Callable>::type;
  }
}

#include <util-generic/callable_signature.ipp>
