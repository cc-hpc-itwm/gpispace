#include <gspc/util/ndebug.hpp>

#include <boost/test/unit_test.hpp>


  namespace gspc::util
  {
    BOOST_AUTO_TEST_CASE (macro_says_same_thing_as_manual_ifdef)
    {
      bool has_ndebug =
#ifdef NDEBUG
      true
#else
      false
#endif
        ;

      bool macro_says = IFDEF_NDEBUG (true) IFNDEF_NDEBUG (false);

      BOOST_REQUIRE_EQUAL (has_ndebug, macro_says);
    }
  }
