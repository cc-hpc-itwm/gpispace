#include <boost/test/minimal.hpp>

#include <fhgcom/kvs/store.hpp>
#include <fhgcom/kvs/backend.hpp>

int test_main(int, char *[])
{
  using namespace fhg::com::kvs;
  // create max size with cache of size 1
  typedef basic_store<detail::null_backend, detail::cache> store_t;
  detail::null_backend b;
  store_t store (b, 1);

  BOOST_REQUIRE( store.cache_size() == 1 );
  BOOST_REQUIRE( store.num_cached() == 0 );

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

  return boost::exit_success;
}
