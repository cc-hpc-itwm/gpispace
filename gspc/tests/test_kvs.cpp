#define BOOST_TEST_MODULE GspcKvsTests
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <errno.h>
#include <stdlib.h>

#include <algorithm>    // std::sort

#include <fhg/util/now.hpp>
#include <we/type/value/show.hpp>

#include <gspc/net.hpp>
#include <gspc/kvs/impl/kvs_impl.hpp>
#include <gspc/kvs/impl/kvs_net_service.hpp>
#include <gspc/kvs/impl/kvs_net_frontend.hpp>

BOOST_AUTO_TEST_CASE (test_impl_invalid_key)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.get ("foo bar", val);
  BOOST_REQUIRE_EQUAL (rc, -EKEYREJECTED);
}

BOOST_AUTO_TEST_CASE (test_impl_put_get_del)
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

BOOST_AUTO_TEST_CASE (test_impl_push_try_pop)
{
  int rc;
  gspc::kvs::kvs_t kvs;
  gspc::kvs::api_t::value_type val;

  rc = kvs.try_pop ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, -EAGAIN);

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

static void s_push_value ( gspc::kvs::api_t *kvs
                         , gspc::kvs::api_t::key_type const &key
                         , gspc::kvs::api_t::value_type const &val
                         )
{
  usleep (rand () % 1500);
  kvs->push (key, val);
}

BOOST_AUTO_TEST_CASE (test_impl_push_pop)
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

  rc = kvs.pop ("foo", val, 500);
  BOOST_REQUIRE_EQUAL (rc, -ETIME);

  // fire up a thread to push
  gspc::kvs::api_t::value_type val_to_push (std::string ("bar"));
  boost::thread pusher (boost::bind (&s_push_value, &kvs, "foo", val_to_push));

  rc = kvs.pop ("foo", val, 1500);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (boost::get<std::string>(val), "bar");

  pusher.join ();
}

BOOST_AUTO_TEST_CASE (test_impl_reset_inc_dec)
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

  rc = kvs.counter_change ("foo", val, 2);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 3);

  rc = kvs.counter_decrement ("foo", val);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (val, 2);

  rc = kvs.counter_change ("foo", val, -2);
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

BOOST_AUTO_TEST_CASE (test_impl_get_regex)
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

BOOST_AUTO_TEST_CASE (test_impl_del_regex)
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

BOOST_AUTO_TEST_CASE (test_impl_expiry)
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

BOOST_AUTO_TEST_CASE (test_net_get_nokey)
{
  gspc::net::initialize ();

  int rc;
  gspc::kvs::api_t::value_type val;

  // setup server
  gspc::net::server_ptr_t server (gspc::net::serve ("tcp://localhost:*"));
  gspc::kvs::service_t service;
  gspc::net::handle ( "/service/kvs"
                    , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                       , boost::ref (service)
                                                       )
                    );

  gspc::kvs::kvs_net_frontend_t kvs (server->url () + "?timeout=1000");

  try
  {
    rc = kvs.get ("foo", val);
    BOOST_REQUIRE_EQUAL (rc, -ENOKEY);
  }
  catch (std::exception const &)
  {
    server->stop ();
    gspc::net::shutdown ();
    throw;
  }

  server->stop ();
  gspc::net::shutdown ();
}


BOOST_AUTO_TEST_CASE (test_net_api)
{
  gspc::net::initialize ();

  int rc;
  gspc::kvs::api_t::value_type val;

  // setup server
  gspc::net::server_ptr_t server (gspc::net::serve ("tcp://localhost:*"));
  gspc::kvs::service_t service;
  gspc::net::handle ( "/service/kvs"
                    , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                       , boost::ref (service)
                                                       )
                    );

  std::cerr << "server running on: " << server->url () << std::endl;

  gspc::kvs::kvs_net_frontend_t kvs (server->url () + "?timeout=1000");

  try
  {
    rc = kvs.get ("foo", val);
    BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

    rc = kvs.put ("foo", "bar");
    BOOST_REQUIRE_EQUAL (rc, 0);

    rc = kvs.get ("foo", val);
    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE_EQUAL (boost::get<std::string>(val), "bar");

    rc = kvs.set_ttl ("foo", 60);
    BOOST_REQUIRE_EQUAL (rc, 0);

    rc = kvs.set_ttl_regex ("foo", 60);
    BOOST_REQUIRE_EQUAL (rc, 0);

    rc = kvs.push ("bar", "foo");
    BOOST_REQUIRE_EQUAL (rc, 0);

    rc = kvs.pop ("bar", val);
    BOOST_REQUIRE_EQUAL (boost::get<std::string>(val), "foo");

    rc = kvs.try_pop ("bar", val);
    BOOST_REQUIRE_EQUAL (rc, -EAGAIN);

    rc = kvs.try_pop ("bar", val);
    BOOST_REQUIRE_EQUAL (rc, -EAGAIN);

    int cnt;
    rc = kvs.counter_reset ("cnt", 0);
    BOOST_REQUIRE_EQUAL (rc, 0);

    rc = kvs.counter_change ("cnt", cnt, +1);
    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE_EQUAL (cnt, 1);

    rc = kvs.counter_change ("cnt", cnt, -1);
    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE_EQUAL (cnt, 0);

    rc = kvs.del_regex (".*");
    BOOST_REQUIRE_EQUAL (rc, 0);

    rc = kvs.get ("bar", val);
    BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

    rc = kvs.del ("cnt");
    BOOST_REQUIRE_EQUAL (rc, -ENOKEY);

    rc = kvs.del ("foo");
    BOOST_REQUIRE_EQUAL (rc, -ENOKEY);
  }
  catch (...)
  {
    server->stop ();
    gspc::net::shutdown ();
    throw;
  }

  server->stop ();
  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_net_put_get)
{
  gspc::net::initialize ();

  int rc;
  gspc::kvs::api_t::value_type val;

  // setup server
  gspc::net::server_ptr_t server (gspc::net::serve ("tcp://localhost:*"));
  gspc::kvs::service_t service;
  gspc::net::handle ( "/service/kvs"
                    , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                       , boost::ref (service)
                                                       )
                    );

  std::cerr << "server running on: " << server->url () << std::endl;

  gspc::kvs::kvs_net_frontend_t kvs (server->url () + "?timeout=1000");

  try
  {
    static const size_t NUM = 1 << 15;

    {
      double duration = -fhg::util::now ();
      for (size_t i = 0 ; i < NUM; ++i)
      {
        rc = kvs.put ("foo", std::string ("bar"));
        BOOST_REQUIRE_EQUAL (rc, 0);

        rc = kvs.get ("foo", val);
        BOOST_REQUIRE_EQUAL (rc, 0);
      }
      duration += fhg::util::now ();

      std::cerr << "put/get of " << NUM << " elements took: "
                << duration << " sec"
                << " => " << 2*(NUM / duration) << " ops/sec"
                << std::endl;
    }
  }
  catch (...)
  {
    server->stop ();
    gspc::net::shutdown ();
    throw;
  }

  server->stop ();
  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_net_push_pop)
{
  gspc::net::initialize ();

  int rc;
  gspc::kvs::api_t::value_type val;

  // setup server
  gspc::net::server_ptr_t server (gspc::net::serve ("tcp://localhost:*"));
  gspc::kvs::service_t service;
  gspc::net::handle ( "/service/kvs"
                    , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                       , boost::ref (service)
                                                       )
                    );

  std::cerr << "server running on: " << server->url () << std::endl;

  gspc::kvs::kvs_net_frontend_t kvs (server->url () + "?timeout=1000");

  try
  {
    // fire up a thread to push
    gspc::kvs::api_t::value_type val_to_push (std::string ("bar"));
    boost::thread pusher (boost::bind ( &s_push_value
                                      , &kvs
                                      , "foo"
                                      , val_to_push
                                      )
                         );

    rc = kvs.pop ("foo", val, 1500);
    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE_EQUAL (boost::get<std::string>(val), "bar");

    pusher.join ();
  }
  catch (...)
  {
    server->stop ();
    gspc::net::shutdown ();
    throw;
  }

  server->stop ();
  gspc::net::shutdown ();
}
