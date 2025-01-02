// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/testing/printer/generic.hpp>

#include <string>
#include <type_traits>

//! \note c++17 but most compilers already have support
#undef FHG_UTIL_TESTING_HAVE_CXXABI
#if defined (__has_include)
#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#define FHG_UTIL_TESTING_HAVE_CXXABI
#endif
#endif

namespace fhg
{
  namespace util
  {
    namespace testing
    {
#define FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL(...)                        \
      FHG_UTIL_TESTING_CHECK_TYPE_IMPL (REQUIRE, "!=", !!, __VA_ARGS__)
#define FHG_UTIL_TESTING_CHECK_TYPE_EQUAL(...)                          \
      FHG_UTIL_TESTING_CHECK_TYPE_IMPL (CHECK, "!=", !!, __VA_ARGS__)

#define FHG_UTIL_TESTING_REQUIRE_TYPE_NE(...)                           \
      FHG_UTIL_TESTING_CHECK_TYPE_IMPL (REQUIRE, "==", !, __VA_ARGS__)
#define FHG_UTIL_TESTING_CHECK_TYPE_NE(...)                             \
      FHG_UTIL_TESTING_CHECK_TYPE_IMPL (CHECK, "==", !, __VA_ARGS__)

      namespace detail
      {
#define FHG_UTIL_TESTING_CHECK_TYPE_IMPL(level_, failed_op_, unary_, ...) \
        do                                                                \
        {                                                                 \
          fhg::util::testing::detail::check_type_impl<__VA_ARGS__>        \
            fhg_util_testing_check_type_impl_checker {failed_op_};        \
          BOOST_ ## level_ ## _MESSAGE                                    \
            ( unary_ fhg_util_testing_check_type_impl_checker             \
            , fhg_util_testing_check_type_impl_checker                    \
            );                                                            \
        }                                                                 \
        while (false)

        template<typename T>
          void show_pretty_typename (std::ostream& os)
        {
#ifdef FHG_UTIL_TESTING_HAVE_CXXABI
          int status;
          std::string mangled_name (typeid (T).name());
          const char* raw_demangled_name
            (abi::__cxa_demangle (mangled_name.c_str(), nullptr, nullptr, &status));
          const std::string demangled_name
            (status == 0 ? raw_demangled_name : mangled_name);
          os << demangled_name;
#else
          os << typeid (T).name();
#endif
        }

        template<typename T, typename U>
          struct check_type_impl
        {
          char const* const _op;
          check_type_impl (char const* const op) : _op (op) {}

          explicit operator bool() const
          {
            return std::is_same<T, U>{};
          }
        };

        template<typename T, typename U>
          std::ostream& operator<<
            (std::ostream& os, check_type_impl<T, U> const& c)
        {
          show_pretty_typename<T> (os);
          os << " " << c._op << " ";
          show_pretty_typename<U> (os);
          return os;
        }
      }
    }
  }
}
