// Copyright (C) 2019,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once




      namespace gspc::testing::detail
      {
        //! \note Customization point. Overload or partially
        //! specialize for T, enabled by void_t or alike. Implement
        //! `T operator()() const`.
        //! \note See gspc/testing/random/xxx.hpp for built-in
        //! specializations. These are automatically included, but
        //! moved there for readability.
        template<typename T, typename = void> struct random_impl {};

        //! Helper to specialize random_impl for <type_, void> and
        //! start defining the function.
        //! \note Must be used in global namespace.
#define GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE(type_)              \
        GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE_IMPL (type_)

        //! Helper to specialize random_impl for template<templ_>
        //! … <type_, cond_> and start defining the function.
        //! \note Must be used in global namespace.
#define GSPC_TESTING_RANDOM_SPECIALIZE_FULL(type_, cond_, templ_...) \
        GSPC_TESTING_RANDOM_SPECIALIZE_FULL_IMPL (type_, cond_, templ_)
      }




#include <gspc/testing/random/impl.ipp>
