// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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
