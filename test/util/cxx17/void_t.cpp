#include <gspc/util/cxx17/void_t.hpp>
#include <gspc/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>



    namespace gspc::util::cxx17
    {
      BOOST_AUTO_TEST_CASE (empty)
      {
        GSPC_TESTING_REQUIRE_TYPE_EQUAL (void_t<>, void);
      }

      BOOST_AUTO_TEST_CASE (void_)
      {
        GSPC_TESTING_REQUIRE_TYPE_EQUAL (void_t<void>, void);
      }

      BOOST_AUTO_TEST_CASE (void_void_void_void_void_void)
      {
        GSPC_TESTING_REQUIRE_TYPE_EQUAL
          (void_t<void, void, void, void, void, void>, void);
      }

      BOOST_AUTO_TEST_CASE (any_type)
      {
        GSPC_TESTING_REQUIRE_TYPE_EQUAL
          (void_t<int, float, std::string, void_t<int>>, void);
      }
    }
