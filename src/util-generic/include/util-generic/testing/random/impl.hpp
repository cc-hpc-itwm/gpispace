// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        //! \note Customization point. Overload or partially
        //! specialize for T, enabled by void_t or alike. Implement
        //! `T operator()() const`.
        //! \note See util-generic/testing/random/xxx.hpp for built-in
        //! specializations. These are automatically included, but
        //! moved there for readability.
        template<typename T, typename = void> struct random_impl {};

        //! Helper to specialize random_impl for <type_, void> and
        //! start defining the function.
        //! \note Must be used in global namespace.
#define FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE(type_)              \
        FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE_IMPL (type_)

        //! Helper to specialize random_impl for template<templ_>
        //! … <type_, cond_> and start defining the function.
        //! \note Must be used in global namespace.
#define FHG_UTIL_TESTING_RANDOM_SPECIALIZE_FULL(type_, cond_, templ_...) \
        FHG_UTIL_TESTING_RANDOM_SPECIALIZE_FULL_IMPL (type_, cond_, templ_)
      }
    }
  }
}

#include <util-generic/testing/random/impl.ipp>
