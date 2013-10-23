#define BOOST_TEST_MODULE GspcKvsTests
#include <boost/test/unit_test.hpp>

#include <errno.h>

#include <algorithm>    // std::sort
#include <gspc/kvs/impl/kvs_impl.hpp>

BOOST_AUTO_TEST_CASE (test_kvs_impl_invalid_key)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.get ("foo bar", val);
  BOOST_REQUIRE_EQUAL (rc, -EKEYREJECTED);
}

BOOST_AUTO_TEST_CASE (test_kvs_impl_put_get_del)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.get ("foo", val);

  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

  rc = kvs.put ("foo", 42);

  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.get ("foo", val);

  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 42);

  rc = kvs.put ("foo", std::string ("bar"));
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.get ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);

  BOOST_REQUIRE_EQUAL ( std::string (boost::get<std::string>(val))
                      , "bar"
                      );

  rc = kvs.del ("foo");

  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.del ("foo");

  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

  rc = kvs.get ("foo", val);

  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);
}

BOOST_AUTO_TEST_CASE (test_kvs_impl_push_try_pop)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

  rc = kvs.put ("foo", 42);
  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -EINVAL);

  rc = kvs.del ("foo");
  BOOST_REQUIRE_EQUAL (rc, 0);

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

BOOST_AUTO_TEST_CASE (test_kvs_impl_push_pop)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.push ("foo", 1);
  BOOST_REQUIRE_EQUAL (rc, 0);
  rc = kvs.push ("foo", 2);
  BOOST_REQUIRE_EQUAL (rc, 0);
  rc = kvs.push ("foo", 3);
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 1);

  rc = kvs.pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 2);

  rc = kvs.pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<int>(val), 3);

  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -EAGAIN);
}

BOOST_AUTO_TEST_CASE (test_kvs_impl_reset_inc_dec)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  int val;

  rc = kvs.counter_increment ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

  rc = kvs.counter_decrement ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

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

struct compare_first
{
  template <typename T>
  bool operator()(T const &a, T const &b)
  {
    return a.first < b.first;
  }
};

BOOST_AUTO_TEST_CASE (test_kvs_impl_get_regex)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  std::list<std::pair< gspc::kvs::kvs_t::key_type
                     , gspc::kvs::kvs_t::value_type
                     > > values;

  kvs.put ("foo.1", "bar");
  kvs.put ("foo.2", "bar");
  kvs.put ("foo.3", "bar");
  kvs.put ("foo.bar", "bar");

  rc = kvs.get_regex ("^foo\\.[0-9]$", values);
  BOOST_REQUIRE_EQUAL (rc, 0);

  BOOST_REQUIRE_EQUAL (values.size (), 3u);

  std::list<std::pair< gspc::kvs::kvs_t::key_type
                     , gspc::kvs::kvs_t::value_type
                     > >::const_iterator it = values.begin ();
  std::list<std::pair< gspc::kvs::kvs_t::key_type
                     , gspc::kvs::kvs_t::value_type
                     > >::const_iterator end = values.end ();

  while (it != end)
  {
    if (it->first == "foo.bar")
      BOOST_ERROR ("key 'foo.bar' must not be in the result set");
    ++it;
  }
}

BOOST_AUTO_TEST_CASE (test_kvs_impl_del_regex)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::kvs_t::value_type val;

  kvs.put ("foo.1", "bar");
  kvs.put ("foo.2", "bar");
  kvs.put ("foo.3", "bar");
  kvs.put ("foo.bar", std::string ("bar"));

  rc = kvs.del_regex ("^foo\\.[0-9]$");
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = kvs.get ("foo.1", val);
  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);
  rc = kvs.get ("foo.2", val);
  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);
  rc = kvs.get ("foo.3", val);
  BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

  rc = kvs.get ("foo.bar", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE (boost::get<std::string>(val) == "bar");
}

BOOST_AUTO_TEST_CASE (test_kvs_impl_expiry)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  kvs.put ("foo.1", "bar");
  kvs.set_ttl ("foo.1", 1);

  sleep (1);

  rc = kvs.get ("foo.1", val);
  BOOST_REQUIRE_EQUAL (rc, -EKEYEXPIRED);
}
