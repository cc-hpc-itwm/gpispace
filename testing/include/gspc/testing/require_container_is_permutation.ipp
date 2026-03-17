#include <gspc/testing/printer/generic.hpp>

#include <boost/test/tools/context.hpp>
#include <boost/test/tools/interface.hpp>

#include <algorithm>

#define GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION_IMPL(lhs_, rhs_)         \
  do                                                                           \
  {                                                                            \
    /* \note Doesn't use a function for new scope to keep source location */   \
    /* in the error message, but should also to allow for expressions as */    \
    /* arguments, so requires a temporary. lhs and rhs are quite common, so */ \
    /* so prefix them with the macro name. */                                  \
    auto const gtrcipi_lhs (lhs_);                                             \
    auto const gtrcipi_rhs (rhs_);                                             \
    BOOST_TEST_CONTEXT                                                         \
      (#lhs_ " = " << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (gtrcipi_lhs))    \
    BOOST_TEST_CONTEXT                                                         \
      (#rhs_ " = " << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (gtrcipi_rhs))    \
    {                                                                          \
      BOOST_REQUIRE_EQUAL (gtrcipi_lhs.size(), gtrcipi_rhs.size());            \
      BOOST_REQUIRE_MESSAGE                                                    \
        ( std::is_permutation                                                  \
            (gtrcipi_lhs.begin(), gtrcipi_lhs.end(), gtrcipi_rhs.begin())      \
        , "the elements shall be a permutation of the expected value"          \
        );                                                                     \
    }                                                                          \
  }                                                                            \
  while (false)
