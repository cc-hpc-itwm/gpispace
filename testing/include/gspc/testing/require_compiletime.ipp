#pragma once

#include <gspc/testing/printer/generic.hpp>

#include <iosfwd>




      namespace gspc::testing::detail
      {
#define GSPC_TESTING_COMPILETIME_CHECK_IMPL(level_, condition_...)      \
        do                                                              \
        {                                                               \
          constexpr bool gspc_testing_check_compiletime_impl            \
            = condition_;                                               \
          BOOST_ ## level_ ## _MESSAGE                                  \
            (gspc_testing_check_compiletime_impl, # condition_);        \
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
          return os << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (p._lhs)  \
                    << " " << # inverse_operator_ << " "                \
                    << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (p._rhs); \
        }

        MAKE_COMPARATOR (is_equal, ==, !=)
        MAKE_COMPARATOR (is_not_equal, !=, ==)
        MAKE_COMPARATOR (is_less, <, >=)
        MAKE_COMPARATOR (is_less_equal, <=, >)
        MAKE_COMPARATOR (is_greater, >, <=)
        MAKE_COMPARATOR (is_greater_equal, >=, <)

#undef MAKE_COMPARATOR

#define GSPC_TESTING_COMPILETIME_RELATION_IMPL(level_, checker_, elems_...)    \
        do                                                              \
        {                                                               \
          constexpr bool gspc_testing_check_compiletime_impl            \
            = gspc::testing::detail::checker_ (elems_);                 \
          BOOST_ ## level_ ## _MESSAGE                                  \
            ( gspc_testing_check_compiletime_impl                       \
            , # checker_ " (" # elems_ ") failed: "                     \
            << gspc::testing::detail::checker_ ## _printer (elems_)     \
            );                                                          \
        }                                                               \
        while (false)
      }
