#include <gspc/testing/printer/chrono.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/test/unit_test.hpp>

#include <chrono>
#include <functional>
#include <type_traits>

#define GSPC_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME_IMPL(timeout_)        \
  using BOOST_PP_CAT (gtrmrt_Duration_, __LINE__)                       \
    = typename std::decay<decltype (timeout_)>::type;                   \
  struct                                                                \
  {                                                                     \
    BOOST_PP_CAT (gtrmrt_Duration_, __LINE__) const allowed;            \
    void operator>> (std::function<void()> fun)                         \
    {                                                                   \
      auto const start (std::chrono::steady_clock::now());              \
      fun();                                                            \
      auto const end (std::chrono::steady_clock::now());                \
      BOOST_REQUIRE_LT (end - start, allowed);                          \
    }                                                                   \
  } BOOST_PP_CAT (gtrmrt_, __LINE__) = {timeout_};                      \
  BOOST_PP_CAT (gtrmrt_, __LINE__) >> [&]
