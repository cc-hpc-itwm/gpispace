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

#include <util-generic/cxx17/void_t.hpp>
#include <util-generic/serialization/trivial.hpp>

#include <functional>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>

namespace fhg
{
  namespace util
  {
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_IMPL(name_, base_)               \
    struct name_                                                        \
    {                                                                   \
      using underlying_type = base_;                                    \
                                                                        \
    private:                                                            \
      underlying_type value {0};                                        \
                                                                        \
    public:                                                             \
      explicit constexpr name_ (decltype (value) v) : value (v) {}      \
                                                                        \
      name_ (name_ const&) = default;                                   \
      name_ (name_&&) = default;                                        \
      name_& operator= (name_ const&) = default;                        \
      name_& operator= (name_&&) = default;                             \
      name_() = default;                                                \
      ~name_() = default;                                               \
                                                                        \
      name_& operator++() { value++; return *this; }                    \
      name_ operator++ (int) { auto r (*this); ++value; return r; }     \
                                                                        \
      template< typename U                                              \
              , typename                                                \
              = fhg::util::hit_detail::enable_if_cast_allowed<name_, U> \
              > explicit constexpr operator U() const                   \
      {                                                                 \
        return static_cast<U> (value);                                  \
      }                                                                 \
                                                                        \
      explicit constexpr operator bool() const { return !!value; }      \
      constexpr bool operator!= (name_ const& rhs) const                \
      { return value != rhs.value; }                                    \
      constexpr bool operator<  (name_ const& rhs) const                \
      { return value < rhs.value; }                                     \
      constexpr bool operator<= (name_ const& rhs) const                \
      { return value <= rhs.value; }                                    \
      constexpr bool operator== (name_ const& rhs) const                \
      { return value == rhs.value; }                                    \
      constexpr bool operator>  (name_ const& rhs) const                \
      { return value > rhs.value; }                                     \
      constexpr bool operator>= (name_ const& rhs) const                \
      { return value >= rhs.value; }                                    \
                                                                        \
      std::size_t hash_value() const                                    \
      {                                                                 \
        return std::hash<base_>{} (value);                              \
      }                                                                 \
      using hard_typedef_tag = struct {};                               \
      template<typename T, bool b>                                      \
        friend struct fhg::util::hit_detail::access;                    \
                                                                        \
      static_assert ( std::is_integral<base_>{}                         \
                    , "hard typedef base shall be integral"             \
                    );                                                  \
    };                                                                  \
    using fhg::util::hit_detail::to_string

    namespace hit_detail
    {
      template<typename T, typename = void>
        struct is_hit : std::false_type {};
      template<typename T>
        struct is_hit<T, fhg::util::cxx17::void_t<typename T::hard_typedef_tag>>
        : std::true_type {};

      template<typename HIT, typename T = void>
        using enable_if_hit = typename std::enable_if<is_hit<HIT>::value, T>::type;
      template<typename HIT, typename T = void>
        using disable_if_hit = typename std::enable_if<!is_hit<HIT>::value, T>::type;

      template<typename U, typename T> struct const_if_const { using type = T; };
      template<typename U, typename T> struct const_if_const<U const, T> { using type = T const; };

      template<typename HIT, bool = is_hit<HIT>::value> struct access;
      template<typename HIT> struct access<HIT, true>
      {
        constexpr auto operator() (HIT& x) const
          -> typename const_if_const<HIT, decltype (x.value)>::type&
        {
          return x.value;
        }
      };
      template<typename Other> struct access<Other, false>
      {
        constexpr Other& operator() (Other& x) const { return x; }
      };
      template<typename T>
        inline constexpr auto value_of (T& x) -> decltype (access<T>{} (x))
      {
        return access<T>{} (x);
      }

      template<typename HIT>
        inline hit_detail::enable_if_hit<HIT, std::string> to_string (HIT const& x)
      {
        return std::to_string (value_of (x));
      }

      //! \note Compare return value as well as Other may be
      //! implicitly converted and call a different
      //! conversion_allowed_without_extra_implicit_conversion().
      template<typename HIT, typename Other>
        using enable_if_cast_allowed
          = typename std::enable_if
              < std::is_same
                  < decltype
                      ( conversion_allowed_without_extra_implicit_conversion
                          (std::declval<HIT>(), std::declval<Other>())
                      )
                  , Other
                  >{}
              >::type;

      struct force_semicolon_in_function_declaration_macro_hack {};
    }

#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_IMPL(op_, lhs_, rhs_, result_) \
    constexpr result_ operator op_ (lhs_ const& lhs, rhs_ const& rhs)   \
    {                                                                   \
      using fhg::util::hit_detail::value_of;                            \
      using promoted_t = decltype (value_of (lhs) op_ value_of (rhs));  \
      using result_t                                                    \
        = decltype (value_of (std::declval<result_ const&>()));         \
      return result_                                                    \
        ( static_cast<result_t>                                         \
            ( static_cast<promoted_t> (value_of (lhs))                  \
              op_                                                       \
              static_cast<promoted_t> (value_of (rhs))                  \
            )                                                           \
        );                                                              \
    }                                                                   \
    using fhg::util::hit_detail::force_semicolon_in_function_declaration_macro_hack
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN_IMPL(op_, lhs_, rhs_)        \
    inline lhs_& operator op_ ## = (lhs_& lhs, rhs_ const& rhs)         \
    {                                                                   \
      using fhg::util::hit_detail::value_of;                            \
      using promoted_t = decltype (value_of (lhs) op_ value_of (rhs));  \
      using result_t                                                    \
        = decltype (value_of (std::declval<lhs_ const&>()));            \
      value_of (lhs)                                                    \
        = static_cast<result_t>                                         \
            ( static_cast<promoted_t> (value_of (lhs))                  \
              op_                                                       \
              static_cast<promoted_t> (value_of (rhs))                  \
            );                                                          \
      return lhs;                                                       \
    }                                                                   \
    using fhg::util::hit_detail::force_semicolon_in_function_declaration_macro_hack

    //! \note Desn't actually list any members since the only member
    //! is private. It is asserted to be an integral type though, so
    //! trivial by definition.
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION_IMPL(T_)  \
    FHG_UTIL_SERIALIZATION_TRIVIAL (T_)

#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_FUN(hit_, fun_) \
    static constexpr hit_                                                    \
      fun_() noexcept (noexcept (_underlying_limits::fun_()))                \
    {                                                                        \
      return hit_ {_underlying_limits::fun_()};                              \
    }
#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR(hit_, var_) \
    static constexpr decltype (_underlying_limits::var_)                     \
      var_ = _underlying_limits::var_
  }
}

#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL(hit_)           \
static_assert ( fhg::util::hit_detail::is_hit<hit_>{}                        \
              , #hit_ " shall be a HardIntegralTypedef"                      \
              );                                                             \
namespace std                                                                \
{                                                                            \
  template<> struct numeric_limits<hit_>                                     \
  {                                                                          \
    using _underlying_limits                                                 \
      = std::numeric_limits<hit_::underlying_type>;                          \
                                                                             \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_FUN                 \
      (hit_, min)                                                            \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_FUN                 \
      (hit_, lowest)                                                         \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_FUN                 \
      (hit_, max)                                                            \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_FUN                 \
      (hit_, epsilon)                                                        \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_FUN                 \
      (hit_, round_error)                                                    \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_FUN                 \
      (hit_, infinity)                                                       \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_FUN                 \
      (hit_, quiet_NaN)                                                      \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_FUN                 \
      (hit_, signaling_NaN)                                                  \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_FUN                 \
      (hit_, denorm_min)                                                     \
                                                                             \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, is_specialized);                                                \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, is_signed);                                                     \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, is_integer);                                                    \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, is_exact);                                                      \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, has_infinity);                                                  \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, has_quiet_NaN);                                                 \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, has_signaling_NaN);                                             \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, has_denorm);                                                    \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, has_denorm_loss);                                               \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, round_style);                                                   \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, is_iec559);                                                     \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, is_bounded);                                                    \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, is_modulo);                                                     \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, digits);                                                        \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, digits10);                                                      \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, max_digits10);                                                  \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, radix);                                                         \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, min_exponent);                                                  \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, min_exponent10);                                                \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, max_exponent);                                                  \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, max_exponent10);                                                \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, traps);                                                         \
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS_DETAIL_VAR                 \
      (hit_, tinyness_before);                                               \
  };                                                                         \
}

#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH_IMPL(name_)         \
namespace std                                                   \
{                                                               \
  template<> struct hash<name_>                                 \
  {                                                             \
    fhg::util::hit_detail::enable_if_hit<name_, size_t>         \
      operator() (name_ const& x) const                         \
    {                                                           \
      return x.hash_value();                                    \
    }                                                           \
  };                                                            \
}

#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ISTREAM_OPERATOR_IMPL(name_)     \
  inline std::istream& operator>> (std::istream& is, name_& x)          \
  {                                                                     \
    using fhg::util::hit_detail::value_of;                              \
    return is >> value_of (x);                                          \
  }                                                                     \
  using fhg::util::hit_detail::force_semicolon_in_function_declaration_macro_hack

#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_OSTREAM_OPERATOR_IMPL(name_)     \
  inline std::ostream& operator<< (std::ostream& os, name_ const& x)    \
  {                                                                     \
    using fhg::util::hit_detail::value_of;                              \
    return os << value_of (x);                                          \
  }                                                                     \
  using fhg::util::hit_detail::force_semicolon_in_function_declaration_macro_hack
