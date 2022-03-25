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

#include <util-generic/va_args.hpp>

namespace fhg
{
  namespace util
  {
    //! `FHG_UTIL_HARD_INTEGRAL_TYPEDEF (T, integral-type);` provides
    //! a type `T` with
    //!
    //! * typename T::underlying_type = integral-type
    //! * explicit ctor from integral-type
    //! * defaulted other ctors
    //!
    //! * explicit conversion to integral-type
    //! * explicit operator bool
    //! * !=, <, <=, ==, >, >=
    //! * ++t, t++
    //!
    //! * to_string (T)
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF(name_, base_)                    \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_IMPL (name_, base_)

    //! Provides `result_ operator op_ (lhs_, rhs_)`. `rhs_` and
    //! `result_` defaults to `lhs_` if not specified. Has to be used
    //! in a namespace qualifying for ADL of `lhs_` or `rhs_`.
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR(op_, /*lhs, rhs = lhs, result = lhs*/...) \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_IMPL (op_, FHG_UTIL_REQUIRED_VA_ARG (1, __VA_ARGS__), FHG_UTIL_OPTIONAL_VA_ARG (2, 1, __VA_ARGS__), FHG_UTIL_OPTIONAL_VA_ARG (3, 1, __VA_ARGS__))

    //! Provides `lhs_& operator op_= (lhs_&, rhs_)`. `rhs_` defaults
    //! to `lhs_` if not specified. Has to be used in a namespace
    //! qualifying for ADL of `lhs_` or `rhs_`.
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN(op_, /*lhs, rhs = lhs*/...) \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN_IMPL (op_, FHG_UTIL_REQUIRED_VA_ARG (1, __VA_ARGS__), FHG_UTIL_OPTIONAL_VA_ARG (2, 1, __VA_ARGS__))

    //! Convenience to define `operator op_` and `operator op_=` at
    //! once. `rhs_` and `result_` defaults to `lhs_` if not
    //! specified. Has to be used in a namespace qualifying for ADL of
    //! `lhs_` or `rhs_`.
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATORS(op_, /*lhs, rhs = lhs, result = lhs*/...) \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (op_, __VA_ARGS__);       \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN (op_, __VA_ARGS__)

    //! Provide `std::hash<T>`. Has to be used in global namespace.
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH(name_)    \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH_IMPL(name_)

    //! Provide `std::numeric_limits<T>`. Has to be used in global
    //! namespace.
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS(name_)    \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL (name_)

    //! Allow explicit conversion operator to U. Has to be used in
    //! a namespace qualifying to ADL of T or U.
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ALLOW_CONVERSION(T_, U_)         \
    constexpr U_ conversion_allowed_without_extra_implicit_conversion (T_, U_)

    //! Provide boost based serialization for `T_`. Has to be used in
    //! global namespace.
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION(T_)  \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION_IMPL (T_)

    //! provide input stream operator>> (std::istream&, T&) as
    //! inverse function of `to_string`. Has to be used
    //! in a namespace qualifying for ADL of `T_`.
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ISTREAM_OPERATOR(T_)    \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ISTREAM_OPERATOR_IMPL(T_)

    //! provide output stream operator<< (std::ostream&, T&) using
    //! `to_string`. Has to be used in a namespace qualifying for ADL
    //! of `T_`.
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_OSTREAM_OPERATOR(T_)    \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_OSTREAM_OPERATOR_IMPL(T_)
  }
}

#include <util-generic/hard_integral_typedef.ipp>
