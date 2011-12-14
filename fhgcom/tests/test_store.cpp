#define BOOST_TEST_MODULE StoreTest
#include <boost/test/unit_test.hpp>

#include <fhgcom/kvs/store.hpp>
#include <fhgcom/kvs/backend.hpp>

BOOST_AUTO_TEST_CASE ( output_test )
{
  using namespace fhg::com::kvs;
  // create max size with cache of size 1
  typedef basic_store<detail::null_backend, detail::cache> store_t;
  detail::null_backend b;
  store_t store (b, 1);

  BOOST_CHECK_EQUAL( store.cache_size(), 1U );
  BOOST_CHECK_EQUAL( store.num_cached(), 0U );

  put (store, "fhgcom.tests.test_store.i", "42");
  BOOST_REQUIRE( store.num_cached() == 1 );
  {
    int i (get<int> (store, "fhgcom.tests.test_store.i"));
    BOOST_REQUIRE( i == 42 );
  }

  put (store, "fhgcom.tests.test_store.j", "23");
  BOOST_REQUIRE( store.num_cached() == 1 );
  {
    int j (get<int> (store, "fhgcom.tests.test_store.j"));
    BOOST_REQUIRE( j == 23 );
  }

  try
  {
    get<int> (store, "fhgcom.tests.test_store.i");
    BOOST_FAIL( "expected no_such exception" );
  }
  catch (std::exception const &)
  {
    // expected
  }
}
