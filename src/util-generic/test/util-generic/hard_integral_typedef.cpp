// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <util-generic/hard_integral_typedef.hpp>

#include <util-generic/testing/printer/hard_integral_typedef.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_compiletime.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>
#include <util-generic/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>

#include <sstream>

namespace fhg
{
  namespace util
  {
    //! \note hard typedefs not in an anonymous namespace due to clang
    //! issue #28817 "-Wunneeded-member-function for dtor of trivially
    //! destructible class with internal linkage" (see
    //! https://llvm.org/bugs/show_bug.cgi?id=28817)
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (test_t, std::size_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ALLOW_CONVERSION
      (test_t, test_t::underlying_type);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ISTREAM_OPERATOR (test_t);
  }
}

FHG_UTIL_HARD_INTEGRAL_TYPEDEF_LOG_VALUE_PRINTER (fhg::util::test_t)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION (fhg::util::test_t)

namespace fhg
{
  namespace util
  {
    namespace
    {
#define TRAIT_T_U(name_, ...)                                                  \
      template<typename T, typename U, typename = void>                        \
        struct name_ : std::false_type {};                                     \
      template<typename T, typename U>                                         \
        struct name_<T, U, cxx17::void_t<decltype (__VA_ARGS__)>> : std::true_type{}
#define TRAIT_T_U_V(name_, ...)                                                \
      template<typename T, typename U, typename V, typename = void>            \
        struct name_ : std::false_type {};                                     \
      template<typename T, typename U, typename V>                             \
        struct name_<T, U, V, cxx17::void_t<decltype (__VA_ARGS__)>> : std::true_type{}

      TRAIT_T_U ( can_implicitly_convert_to
                , std::declval<U&>() = std::declval<T>()
                );
      TRAIT_T_U ( can_explicitly_convert_to
                , std::declval<U&>() = static_cast<U> (std::declval<T>())
                );

      //! \note helper to allow for commas
#define REQUIRE(...) do { auto check = __VA_ARGS__; BOOST_REQUIRE (check); } while (false)
    }

    BOOST_AUTO_TEST_CASE (auto_generated_operations)
    {
      FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
        (test_t::underlying_type, std::size_t);

      BOOST_REQUIRE_EQUAL (test_t{}, test_t {0});
      {
        test_t a {0};
        BOOST_REQUIRE_EQUAL (a, test_t {0});
        a = test_t {2};
        BOOST_REQUIRE_EQUAL (a, test_t {2});
        BOOST_REQUIRE_EQUAL (++a, test_t {3});
        BOOST_REQUIRE_EQUAL (a, test_t {3});
        BOOST_REQUIRE_EQUAL (a++, test_t {3});
        BOOST_REQUIRE_EQUAL (a, test_t {4});
      }

      BOOST_REQUIRE_EQUAL (test_t {1}, test_t {1});
      BOOST_REQUIRE_GE (test_t {1}, test_t {1});
      BOOST_REQUIRE_GE (test_t {2}, test_t {1});
      BOOST_REQUIRE_GT (test_t {2}, test_t {1});
      BOOST_REQUIRE_LE (test_t {1}, test_t {1});
      BOOST_REQUIRE_LE (test_t {1}, test_t {2});
      BOOST_REQUIRE_LT (test_t {1}, test_t {2});
      BOOST_REQUIRE_NE (test_t {1}, test_t {2});
      BOOST_REQUIRE (test_t {1} != test_t {2});

      REQUIRE (!can_implicitly_convert_to<test_t, bool>::value);
      REQUIRE (can_explicitly_convert_to<test_t, bool>::value);
      BOOST_REQUIRE (test_t {1});
      BOOST_REQUIRE (!test_t {0});

      REQUIRE (!can_implicitly_convert_to<test_t, std::size_t>::value);
      REQUIRE (can_explicitly_convert_to<test_t, std::size_t>::value);
      REQUIRE (!can_implicitly_convert_to<test_t, int>::value);
      REQUIRE (!can_explicitly_convert_to<test_t, int>::value);

      {
        std::size_t value (fhg::util::testing::random<std::size_t>{}());
        test_t t {value};
        BOOST_REQUIRE_EQUAL (static_cast<std::size_t> (t), value);
      }

      BOOST_REQUIRE (serialization::is_trivially_serializable<test_t>{});

      FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
        ({fhg::util::testing::random<test_t>{}()}, test_t);

      {
        std::size_t value (fhg::util::testing::random<std::size_t>{}());
        test_t t {value};
        std::ostringstream ss;
        ss << to_string (t);
        BOOST_REQUIRE_EQUAL (ss.str(), to_string (t));
        BOOST_REQUIRE_EQUAL (std::to_string (value), to_string (t));

        std::istringstream is (ss.str());
        test_t u;
        is >> u;
        BOOST_REQUIRE (is.eof());
        BOOST_REQUIRE (t == u);
      }
    }

    namespace
    {
      TRAIT_T_U_V (has_add, std::declval<V&>() = std::declval<T>() + std::declval<U>());
      TRAIT_T_U_V (has_div, std::declval<V&>() = std::declval<T>() / std::declval<U>());
      TRAIT_T_U_V (has_mod, std::declval<V&>() = std::declval<T>() % std::declval<U>());
      TRAIT_T_U_V (has_mul, std::declval<V&>() = std::declval<T>() * std::declval<U>());
      TRAIT_T_U_V (has_sub, std::declval<V&>() = std::declval<T>() - std::declval<U>());
      TRAIT_T_U_V (has_xor, std::declval<V&>() = std::declval<T>() ^ std::declval<U>());

      TRAIT_T_U (has_addassign, std::declval<T&>() += std::declval<U>());
      TRAIT_T_U (has_divassign, std::declval<T&>() /= std::declval<U>());
      TRAIT_T_U (has_modassign, std::declval<T&>() %= std::declval<U>());
      TRAIT_T_U (has_mulassign, std::declval<T&>() *= std::declval<U>());
      TRAIT_T_U (has_subassign, std::declval<T&>() -= std::declval<U>());
      TRAIT_T_U (has_xorassign, std::declval<T&>() ^= std::declval<U>());
    }

    BOOST_AUTO_TEST_CASE (does_not_auto_generate_arithmetic_operations)
    {
      REQUIRE (!has_add<std::size_t, test_t, test_t>::value);
      REQUIRE (!has_add<test_t, std::size_t, test_t>::value);
      REQUIRE (!has_add<test_t, test_t, test_t>::value);
      REQUIRE (!has_addassign<test_t, test_t>::value);
      REQUIRE (!has_addassign<test_t, std::size_t>::value);
      REQUIRE (!has_addassign<std::size_t, test_t>::value);

      REQUIRE (!has_div<std::size_t, test_t, test_t>::value);
      REQUIRE (!has_div<test_t, std::size_t, test_t>::value);
      REQUIRE (!has_div<test_t, test_t, test_t>::value);
      REQUIRE (!has_divassign<test_t, test_t>::value);
      REQUIRE (!has_divassign<test_t, std::size_t>::value);
      REQUIRE (!has_divassign<std::size_t, test_t>::value);

      REQUIRE (!has_mod<std::size_t, test_t, test_t>::value);
      REQUIRE (!has_mod<test_t, std::size_t, test_t>::value);
      REQUIRE (!has_mod<test_t, test_t, test_t>::value);
      REQUIRE (!has_modassign<test_t, test_t>::value);
      REQUIRE (!has_modassign<test_t, std::size_t>::value);
      REQUIRE (!has_modassign<std::size_t, test_t>::value);

      REQUIRE (!has_mul<std::size_t, test_t, test_t>::value);
      REQUIRE (!has_mul<test_t, std::size_t, test_t>::value);
      REQUIRE (!has_mul<test_t, test_t, test_t>::value);
      REQUIRE (!has_mulassign<test_t, test_t>::value);
      REQUIRE (!has_mulassign<test_t, std::size_t>::value);
      REQUIRE (!has_mulassign<std::size_t, test_t>::value);

      REQUIRE (!has_sub<std::size_t, test_t, test_t>::value);
      REQUIRE (!has_sub<test_t, std::size_t, test_t>::value);
      REQUIRE (!has_sub<test_t, test_t, test_t>::value);
      REQUIRE (!has_subassign<test_t, test_t>::value);
      REQUIRE (!has_subassign<test_t, std::size_t>::value);
      REQUIRE (!has_subassign<std::size_t, test_t>::value);

      REQUIRE (!has_xor<std::size_t, test_t, test_t>::value);
      REQUIRE (!has_xor<test_t, std::size_t, test_t>::value);
      REQUIRE (!has_xor<test_t, test_t, test_t>::value);
      REQUIRE (!has_xorassign<test_t, test_t>::value);
      REQUIRE (!has_xorassign<test_t, std::size_t>::value);
      REQUIRE (!has_xorassign<std::size_t, test_t>::value);
  }

    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (arith_ops_t, std::size_t);

    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, arith_ops_t);
    BOOST_AUTO_TEST_CASE (arith_operator_only_defines_binary_not_assigning)
    {
      REQUIRE (!has_add<std::size_t, arith_ops_t, std::size_t>::value);
      REQUIRE (!has_add<arith_ops_t, std::size_t, arith_ops_t>::value);
      REQUIRE (has_add<arith_ops_t, arith_ops_t, arith_ops_t>::value);
      REQUIRE (!has_addassign<arith_ops_t, arith_ops_t>::value);
      REQUIRE (!has_addassign<arith_ops_t, std::size_t>::value);
      REQUIRE (!has_addassign<std::size_t, arith_ops_t>::value);
    }

    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN (-, arith_ops_t);
    BOOST_AUTO_TEST_CASE (arith_operator_assign_doesnt_define_binary)
    {
      REQUIRE (!has_sub<std::size_t, arith_ops_t, std::size_t>::value);
      REQUIRE (!has_sub<arith_ops_t, std::size_t, arith_ops_t>::value);
      REQUIRE (!has_sub<arith_ops_t, arith_ops_t, arith_ops_t>::value);
      REQUIRE (has_subassign<arith_ops_t, arith_ops_t>::value);
      REQUIRE (!has_subassign<arith_ops_t, std::size_t>::value);
      REQUIRE (!has_subassign<std::size_t, arith_ops_t>::value);
    }

    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATORS (*, arith_ops_t);
    BOOST_AUTO_TEST_CASE (arith_operators_defines_binary_and_assign)
    {
      REQUIRE (!has_mul<std::size_t, arith_ops_t, std::size_t>::value);
      REQUIRE (!has_mul<arith_ops_t, std::size_t, arith_ops_t>::value);
      REQUIRE (has_mul<arith_ops_t, arith_ops_t, arith_ops_t>::value);
      REQUIRE (has_mulassign<arith_ops_t, arith_ops_t>::value);
      REQUIRE (!has_mulassign<arith_ops_t, std::size_t>::value);
      REQUIRE (!has_mulassign<std::size_t, arith_ops_t>::value);
    }

    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATORS (/, arith_ops_t, std::size_t);
    BOOST_AUTO_TEST_CASE (arith_operators_can_have_different_rhs)
    {
      REQUIRE (!has_div<std::size_t, arith_ops_t, std::size_t>::value);
      REQUIRE (has_div<arith_ops_t, std::size_t, arith_ops_t>::value);
      REQUIRE (!has_div<arith_ops_t, arith_ops_t, arith_ops_t>::value);
      REQUIRE (!has_divassign<arith_ops_t, arith_ops_t>::value);
      REQUIRE (has_divassign<arith_ops_t, std::size_t>::value);
      REQUIRE (!has_divassign<std::size_t, arith_ops_t>::value);
    }

    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATORS (%, std::size_t, arith_ops_t);
    BOOST_AUTO_TEST_CASE (arith_operators_can_have_different_lhs)
    {
      REQUIRE (has_mod<std::size_t, arith_ops_t, std::size_t>::value);
      REQUIRE (!has_mod<arith_ops_t, std::size_t, arith_ops_t>::value);
      REQUIRE (!has_mod<arith_ops_t, arith_ops_t, arith_ops_t>::value);
      REQUIRE (!has_modassign<arith_ops_t, arith_ops_t>::value);
      REQUIRE (!has_modassign<arith_ops_t, std::size_t>::value);
      REQUIRE (has_modassign<std::size_t, arith_ops_t>::value);
    }

    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATORS (^, std::size_t, arith_ops_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATORS (^, arith_ops_t, std::size_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATORS (^, arith_ops_t);
    BOOST_AUTO_TEST_CASE (arith_operators_can_be_overloaded)
    {
      REQUIRE (has_xor<std::size_t, arith_ops_t, std::size_t>::value);
      REQUIRE (has_xor<arith_ops_t, std::size_t, arith_ops_t>::value);
      REQUIRE (has_xor<arith_ops_t, arith_ops_t, arith_ops_t>::value);
      REQUIRE (has_xorassign<arith_ops_t, arith_ops_t>::value);
      REQUIRE (has_xorassign<arith_ops_t, std::size_t>::value);
      REQUIRE (has_xorassign<std::size_t, arith_ops_t>::value);
    }

    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (arith_ops0_t, std::size_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (arith_ops1_t, std::size_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (arith_ops2_t, std::size_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATORS
      (+, arith_ops0_t, arith_ops1_t, arith_ops2_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATORS
      (*, arith_ops0_t, arith_ops1_t, std::size_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATORS
      (-, arith_ops0_t, std::size_t, arith_ops1_t);
    BOOST_AUTO_TEST_CASE (arith_operators_can_convert_between_types)
    {
      REQUIRE (has_add<arith_ops0_t, arith_ops1_t, arith_ops2_t>::value);
      REQUIRE (has_addassign<arith_ops0_t, arith_ops1_t>::value);

      REQUIRE (has_mul<arith_ops0_t, arith_ops1_t, std::size_t>::value);
      REQUIRE (has_mulassign<arith_ops0_t, arith_ops1_t>::value);

      REQUIRE (has_sub<arith_ops0_t, std::size_t, arith_ops1_t>::value);
      REQUIRE (has_subassign<arith_ops0_t, std::size_t>::value);
    }

    BOOST_AUTO_TEST_CASE (operations_are_constexpr)
    {
      FHG_UTIL_TESTING_COMPILETIME_REQUIRE_EQUAL (test_t{}, test_t {0});
      FHG_UTIL_TESTING_COMPILETIME_REQUIRE_NE (test_t {2}, test_t {1});
      FHG_UTIL_TESTING_COMPILETIME_REQUIRE_LT (test_t {0}, test_t {1});
      FHG_UTIL_TESTING_COMPILETIME_REQUIRE_LE (test_t {0}, test_t {1});
      FHG_UTIL_TESTING_COMPILETIME_REQUIRE_GT (test_t {2}, test_t {1});
      FHG_UTIL_TESTING_COMPILETIME_REQUIRE_GE (test_t {1}, test_t {1});
      FHG_UTIL_TESTING_COMPILETIME_REQUIRE (!test_t {0});
    }

    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (hashable_t, int);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ALLOW_CONVERSION
      (hashable_t, hashable_t::underlying_type);
  }
}

FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (fhg::util::hashable_t)

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (generated_hash_is_equivalent_to_underlying_type)
    {
      auto value (fhg::util::testing::random<hashable_t>{}());
      BOOST_REQUIRE_EQUAL ( std::hash<hashable_t>{} (value)
                          , std::hash<int>{} (static_cast<int> (value))
                          );
    }
  }
}

namespace completely
{
  namespace different
  {
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (tester_t, std::size_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ISTREAM_OPERATOR (tester_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_OSTREAM_OPERATOR (tester_t);

    BOOST_AUTO_TEST_CASE (io_adl_works_for_typedefs_in_any_namespace)
    {
      std::size_t value (fhg::util::testing::random<std::size_t>{}());
      tester_t t {value};
      std::ostringstream ss;
      ss << to_string (t);
      BOOST_REQUIRE_EQUAL (ss.str(), to_string (t));

      std::istringstream is (ss.str());
      tester_t u;
      is >> u;
      BOOST_REQUIRE (is.eof());
      BOOST_REQUIRE (t == u);

      std::ostringstream oss;
      oss << t;
      BOOST_REQUIRE_EQUAL (oss.str(), to_string (t));
    }
  }

  namespace another
  {
    struct integral_type
    {
      integral_type (different::tester_t::underlying_type);
    };
  }
}

#include <util-generic/unused.hpp>

namespace fhg
{
  namespace util
  {
    using completely::different::tester_t;

    BOOST_AUTO_TEST_CASE (conversion_is_not_allowed_if_macro_never_used)
    {
      REQUIRE (!can_explicitly_convert_to<tester_t, float>::value);
    }

    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (different_t, tester_t::underlying_type);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ALLOW_CONVERSION (tester_t, test_t);

    BOOST_AUTO_TEST_CASE
      (conversion_is_not_allowed_between_typedefs_per_default)
    {
      REQUIRE (!can_explicitly_convert_to<tester_t, different_t>::value);
      REQUIRE (can_explicitly_convert_to<tester_t, test_t>::value);
    }
  }
}

namespace completely
{
  namespace different
  {
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ALLOW_CONVERSION (tester_t, int);
  }
}

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (conversion_works_with_alias_at_use)
    {
      using alias = int;
      REQUIRE (can_explicitly_convert_to<tester_t, alias>::value);
    }
  }
}

namespace completely
{
  namespace different
  {
    using alias_double = double;
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ALLOW_CONVERSION (tester_t, alias_double);
  }
}

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (conversion_works_with_alias_at_allowance)
    {
      REQUIRE (can_explicitly_convert_to<tester_t, double>::value);
    }
  }
}

namespace completely
{
  namespace different
  {
    using my_size_t = std::size_t;
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ALLOW_CONVERSION (tester_t, my_size_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ALLOW_CONVERSION (tester_t, std::size_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ALLOW_CONVERSION (tester_t, std::size_t);
  }
}

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE
      (conversion_allowance_can_be_given_multiple_times_including_with_aliases)
    {
      REQUIRE (can_explicitly_convert_to<tester_t, std::size_t>::value);
      REQUIRE ( can_explicitly_convert_to
                  <tester_t, completely::different::my_size_t>::value
              );
    }

    namespace not_qualifying_for_adl
    {
      FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ALLOW_CONVERSION
        (completely::different::tester_t, completely::another::integral_type);
    }

    BOOST_AUTO_TEST_CASE
      (conversion_macro_needs_to_be_used_in_adl_qualifying_namespace)
    {
      REQUIRE ( !can_explicitly_convert_to
                  <tester_t, completely::another::integral_type>::value
              );
    }

    BOOST_AUTO_TEST_CASE (only_explicit_conversion_is_allowed)
    {
      REQUIRE (!can_implicitly_convert_to<tester_t, std::size_t>::value);
      REQUIRE (!can_implicitly_convert_to<tester_t, test_t>::value);
      REQUIRE (!can_implicitly_convert_to<tester_t, double>::value);
    }
  }
}

namespace fhg
{
  namespace util
  {
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (limitable_t, unsigned);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (signed_limitable_t, int);
  }
}

FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS (fhg::util::limitable_t)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS (fhg::util::signed_limitable_t)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_LOG_VALUE_PRINTER (fhg::util::limitable_t)

//! \todo shall static_assert
// FHG_UTIL_HARD_INTEGRAL_TYPEDEF_NUMERIC_LIMITS (void);

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (hit_does_not_specialize_numeric_limits_by_default)
    {
      FHG_UTIL_TESTING_COMPILETIME_REQUIRE
        (!std::numeric_limits<test_t>::is_specialized);
      FHG_UTIL_TESTING_COMPILETIME_REQUIRE
        (std::numeric_limits<limitable_t>::is_specialized);
    }

    BOOST_AUTO_TEST_CASE (hit_numeric_limits_are_basically_sane)
    {
      FHG_UTIL_TESTING_COMPILETIME_REQUIRE
        (std::numeric_limits<limitable_t>::is_integer);
      FHG_UTIL_TESTING_COMPILETIME_REQUIRE
        (std::numeric_limits<limitable_t>::is_exact);

      FHG_UTIL_TESTING_COMPILETIME_REQUIRE
        (!std::numeric_limits<limitable_t>::is_signed);
      FHG_UTIL_TESTING_COMPILETIME_REQUIRE
        (std::numeric_limits<signed_limitable_t>::is_signed);

      FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
        ( decltype (std::numeric_limits<limitable_t>::min())
        , limitable_t
        );

      FHG_UTIL_TESTING_COMPILETIME_REQUIRE_EQUAL
        ( std::numeric_limits<limitable_t>::min()
        , std::numeric_limits<limitable_t>::lowest()
        );

      FHG_UTIL_TESTING_COMPILETIME_REQUIRE_EQUAL
        ( std::numeric_limits<limitable_t>::max()
        , limitable_t {std::numeric_limits<limitable_t::underlying_type>::max()}
        );
    }

    //! \note Issue 36: This fails to compile due to narrowing
    //! conversion as char + char = int.
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (T, char);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, T);

    //! All these should not trigger any warnings. Since we want
    //! multiple combinations, there are multiple blocks here to avoid
    //! duplicate definitions.
    namespace expect_no_warnings_with_int_promotion
    {
      FHG_UTIL_HARD_INTEGRAL_TYPEDEF (uchar, unsigned char);
      FHG_UTIL_HARD_INTEGRAL_TYPEDEF (schar, signed char);

      namespace aa
      {
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN (+, schar, schar);
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN (+, uchar, uchar);
      }
      namespace ab
      {
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN (+, schar, uchar);
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN (+, uchar, schar);
      }

      namespace aaa
      {
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, schar, schar, schar);
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, uchar, uchar, uchar);
      }
      namespace aab
      {
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, schar, schar, uchar);
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, uchar, uchar, schar);
      }
      namespace aba
      {
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, schar, uchar, schar);
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, uchar, schar, uchar);
      }
      namespace abb
      {
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, schar, uchar, uchar);
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, uchar, schar, schar);
      }
    }
    namespace expect_no_warnings_without_int_promotion
    {
      FHG_UTIL_HARD_INTEGRAL_TYPEDEF (uint, unsigned int);
      FHG_UTIL_HARD_INTEGRAL_TYPEDEF (sint, signed int);

      namespace aa
      {
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN (+, sint, sint);
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN (+, uint, uint);
      }
      namespace ab
      {
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN (+, sint, uint);
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR_ASSIGN (+, uint, sint);
      }

      namespace aaa
      {
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, sint, sint, sint);
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, uint, uint, uint);
      }
      namespace aab
      {
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, sint, sint, uint);
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, uint, uint, sint);
      }
      namespace aba
      {
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, sint, uint, sint);
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, uint, sint, uint);
      }
      namespace abb
      {
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, sint, uint, uint);
        FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, uint, sint, sint);
      }
    }
  }
}
