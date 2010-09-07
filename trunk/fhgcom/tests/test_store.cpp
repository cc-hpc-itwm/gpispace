#include <boost/test/minimal.hpp>

#include <fhgcom/kvs/store.hpp>

int test_main(int, char *[])
{
  // create max size with cache of size 1
  fhg::com::kvs::store store (1);

  BOOST_REQUIRE( store.cache_size() == 1 );
  BOOST_REQUIRE( store.num_cached() == 0 );

  fhg::com::kvs::put (store, "fhgcom.tests.test_store.i", "42");
  BOOST_REQUIRE( store.num_cached() == 1 );
  {
    int i (fhg::com::kvs::get<int> (store, "fhgcom.tests.test_store.i"));
    BOOST_REQUIRE( i == 42 );
  }

  fhg::com::kvs::put (store, "fhgcom.tests.test_store.j", "23");
  BOOST_REQUIRE( store.num_cached() == 1 );
  {
    int j (fhg::com::kvs::get<int> (store, "fhgcom.tests.test_store.j"));
    BOOST_REQUIRE( j == 23 );
  }

  try
  {
    fhg::com::kvs::get<int> (store, "fhgcom.tests.test_store.i");
    BOOST_FAIL( "expected no_such exception" );
  }
  catch (std::exception const &)
  {
    // expected
  }

  return boost::exit_success;
}
