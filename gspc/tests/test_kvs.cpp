#define BOOST_TEST_MODULE GspcKvsTests
#include <boost/test/unit_test.hpp>

#include <errno.h>

#include <gspc/kvs/kvs_impl.hpp>

BOOST_AUTO_TEST_CASE (test_kvs_backend_put_get_del)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.get ("foo", val);

  BOOST_REQUIRE_EQUAL (rc, -ESRCH);

  rc = kvs.put ("foo", 42);

  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.get ("foo", val);

  BOOST_REQUIRE_EQUAL (rc, 0);

  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 42);

  rc = kvs.del ("foo");

  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.del ("foo");

  BOOST_REQUIRE_EQUAL (rc, -ESRCH);

  rc = kvs.get ("foo", val);

  BOOST_REQUIRE_EQUAL (rc, -ESRCH);
}

BOOST_AUTO_TEST_CASE (test_kvs_backend_push_try_pop)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -ESRCH);

  rc = kvs.put ("foo", 42);
  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -EINVAL);

  rc = kvs.del ("foo");

  rc = kvs.push ("foo", 1);
  BOOST_REQUIRE_EQUAL (rc, 0);
  rc = kvs.push ("foo", 2);
  BOOST_REQUIRE_EQUAL (rc, 0);
  rc = kvs.push ("foo", 3);
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 1);

  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 2);

  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 3);
}

BOOST_AUTO_TEST_CASE (test_kvs_backend_reset_inc_dec)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  int val;

  rc = kvs.counter_increment ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -ESRCH);

  rc = kvs.counter_decrement ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -ESRCH);

  rc = kvs.counter_reset ("foo", 0);
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.counter_increment ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 1);

  rc = kvs.counter_increment ("foo", val, 2);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 3);

  rc = kvs.counter_decrement ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 2);

  rc = kvs.counter_decrement ("foo", val, 2);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 0);

  rc = kvs.del ("foo");
}
