// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <iosfwd>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
#define FHG_UTIL_TESTING_COMPILETIME_CHECK_IMPL(level_, condition_...)  \
        do                                                              \
        {                                                               \
          constexpr bool fhg_util_testing_check_compiletime_impl        \
            = condition_;                                               \
          BOOST_ ## level_ ## _MESSAGE                                  \
            (fhg_util_testing_check_compiletime_impl, # condition_);    \
        }                                                               \
        while (false)

#define MAKE_COMPARATOR(name_, operator_, inverse_operator_)            \
        template<typename Lhs, typename Rhs>                            \
          constexpr bool name_  (Lhs const& lhs, Rhs const& rhs)        \
        {                                                               \
          return lhs operator_ rhs;                                     \
        }                                                               \
        template<typename Lhs, typename Rhs>                            \
        struct name_ ## _printer_t                                      \
        {                                                               \
          Lhs const& _lhs;                                              \
          Rhs const& _rhs;                                              \
          name_ ## _printer_t (Lhs const& lhs, Rhs const& rhs)          \
            : _lhs (lhs)                                                \
            , _rhs (rhs)                                                \
          {}                                                            \
        };                                                              \
        template<typename Lhs, typename Rhs>                            \
          name_ ## _printer_t<Lhs, Rhs>                                 \
            name_ ## _printer (Lhs const& lhs, Rhs const& rhs)          \
        {                                                               \
          return {lhs, rhs};                                            \
        }                                                               \
                                                                        \
        template<typename Lhs, typename Rhs>                            \
          std::ostream& operator<<                                      \
            (std::ostream& os, name_ ## _printer_t<Lhs, Rhs> const& p)  \
        {                                                               \
          return os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (p._lhs)   \
                    << " " << # inverse_operator_ << " "                \
                    << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (p._rhs);  \
        }

        MAKE_COMPARATOR (is_equal, ==, !=)
        MAKE_COMPARATOR (is_not_equal, !=, ==)
        MAKE_COMPARATOR (is_less, <, >=)
        MAKE_COMPARATOR (is_less_equal, <=, >)
        MAKE_COMPARATOR (is_greater, >, <=)
        MAKE_COMPARATOR (is_greater_equal, >=, <)

#undef MAKE_COMPARATOR

#define FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL(level_, checker_, elems_...) \
        do                                                              \
        {                                                               \
          constexpr bool fhg_util_testing_check_compiletime_impl        \
            = fhg::util::testing::detail::checker_ (elems_);            \
          BOOST_ ## level_ ## _MESSAGE                                  \
            ( fhg_util_testing_check_compiletime_impl                   \
            , # checker_ " (" # elems_ ") failed: "                     \
            << fhg::util::testing::detail::checker_ ## _printer (elems_) \
            );                                                          \
        }                                                               \
        while (false)
      }
    }
  }
}
