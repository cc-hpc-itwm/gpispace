#define BOOST_TEST_MODULE GspcKvsNetTests
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <errno.h>
#include <stdlib.h>

#include <algorithm>    // std::sort

#include <fhg/util/now.hpp>
#include <boost/format.hpp>

#include <we/type/value/peek.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <gspc/net.hpp>

#include <gspc/kvs/api.hpp>
#include <gspc/kvs/util.hpp>
#include <gspc/kvs/impl/kvs_impl.hpp>
#include <gspc/kvs/impl/kvs_net_service.hpp>
#include <gspc/kvs/impl/kvs_net_frontend.hpp>

static void s_push_value ( gspc::kvs::api_t *kvs
                         , gspc::kvs::api_t::key_type const &key
                         , gspc::kvs::api_t::value_type const &val
                         )
{
  usleep (rand () % 1500);
  kvs->push (key, val);
}

static void s_wfh_client_thread ( const size_t rank
                                , gspc::kvs::api_t *kvs
                                , const size_t nmsg
                                , const std::string &queue
                                )
{
  int rc;
  const std::string my_queue ((boost::format ("thread-%1%") % rank).str ());

  for (size_t i = 0 ; i < nmsg ; ++i)
  {
    pnet::type::value::value_type rqst
      = pnet::type::value::read
      ((boost::format ( "Struct [from := \"%1%\", msg := %2%]"
                      ) % my_queue % i
       ).str ());
    pnet::type::value::value_type rply;

    rc = gspc::kvs::query ( *kvs
                          , queue
                          , rqst
                          , my_queue
                          , rply
                          , 10 * 1000
                          );
    if (rc != 0)
    {
      std::cerr << "thread[" << rank << "]: "
                << "could not query #" << i << " from '" << queue << "': "
                << strerror (-rc)
                << std::endl
        ;
      kvs->get (my_queue, rply);
      std::cerr << "thread[" << rank << "]: queue content: "
                << pnet::type::value::show (rply)
                << std::endl
        ;
      break;
    }
  }
}

BOOST_AUTO_TEST_CASE (test_net_start_stop)
{
  gspc::net::initializer _net_init;

  int rc;
  gspc::kvs::api_t::value_type val;

  gspc::net::server_ptr_t server;
  size_t i;
  try
  {
    for (i = 0 ; i < 10 ; ++i)
    {
      // setup server
      server = gspc::net::serve ("tcp://localhost:*");
      gspc::kvs::service_t service;
      gspc::net::handle ( "/service/kvs"
                        , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                           , boost::ref (service)
                                                           )
                        );

      gspc::kvs::kvs_net_frontend_t kvs (server->url ());

      rc = kvs.put ("foo", "bar");
      if (rc != 0)
      {
        std::cerr << "failed to put: " << strerror (-rc) << std::endl;
      }
      BOOST_CHECK_EQUAL (rc, 0);

      rc = kvs.get ("foo", val);
      if (rc != 0)
      {
        std::cerr << "failed to put: " << strerror (-rc) << std::endl;
      }
      BOOST_CHECK_EQUAL (rc, 0);
    }
  }
  catch (std::exception const &ex)
  {
    std::cerr << "failed in iteration " << i << ": " << ex.what ()
              << std::endl;

    throw;
  }
}

BOOST_AUTO_TEST_CASE (test_net_get_nokey)
{
  gspc::net::initializer _net_init;

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

  try
  {
    gspc::kvs::kvs_net_frontend_t kvs (server->url () + "?timeout=5000");

    rc = kvs.get ("foo", val);
    BOOST_REQUIRE_EQUAL (rc, -ENOKEY);
  }
  catch (std::exception const &)
  {
    throw;
  }
}

BOOST_AUTO_TEST_CASE (test_net_api)
{
  gspc::net::initializer _net_init;

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

  try
  {
    gspc::kvs::kvs_net_frontend_t kvs (server->url () + "?timeout=5000");

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
    BOOST_REQUIRE_EQUAL (cnt, 0);

    rc = kvs.counter_change ("cnt", cnt, -1);
    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE_EQUAL (cnt, 1);

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
    throw;
  }
}

BOOST_AUTO_TEST_CASE (test_net_put_get)
{
  gspc::net::initializer _net_init;

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

  try
  {
    gspc::kvs::kvs_net_frontend_t kvs (server->url () + "?timeout=5000");

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
    throw;
  }
}

BOOST_AUTO_TEST_CASE (test_net_wait)
{
  gspc::net::initializer _net_init;

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

  try
  {
    gspc::kvs::kvs_net_frontend_t kvs (server->url () + "?timeout=5000");

    rc = kvs.wait ("foo", gspc::kvs::api_t::E_EXIST, 500);
    BOOST_REQUIRE_EQUAL (rc, -ETIME);

    // fire up a thread to push
    gspc::kvs::api_t::value_type val_to_push (std::string ("bar"));
    boost::thread pusher (boost::bind (&s_push_value, &kvs, "foo", val_to_push));

    rc = kvs.wait ("foo", gspc::kvs::api_t::E_PUSH, 2000);
    if (rc < 0)
      std::cerr << "wait returned: " << strerror (-rc) << std::endl;

    BOOST_REQUIRE (rc & gspc::kvs::api_t::E_PUSH);

    pusher.join ();

    rc = kvs.pop ("foo", val);
    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE_EQUAL (boost::get<std::string>(val), "bar");
  }
  catch (...)
  {
    throw;
  }
}

BOOST_AUTO_TEST_CASE (test_net_push_pop)
{
  gspc::net::initializer _net_init;

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

  try
  {
    gspc::kvs::kvs_net_frontend_t kvs (server->url () + "?timeout=5000");

    // fire up a thread to push
    gspc::kvs::api_t::value_type val_to_push (std::string ("bar"));
    boost::thread pusher (boost::bind ( &s_push_value
                                      , &kvs
                                      , "foo"
                                      , val_to_push
                                      )
                         );

    rc = kvs.pop ("foo", val, 1500);
    if (rc != 0)
    {
      std::cerr << "pop failed: " << strerror (-rc) << std::endl;
    }
    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE_EQUAL (boost::get<std::string>(val), "bar");

    pusher.join ();
  }
  catch (...)
  {
    throw;
  }
}

BOOST_AUTO_TEST_CASE (test_net_many_push_pop)
{
  gspc::net::initializer _net_init;

  int rc;
  gspc::kvs::api_t::value_type val;
  const std::string queue ("wfh");

  static const size_t NUM = 10;
  static const size_t NTHREAD = 15;

  // setup server
  gspc::net::server_ptr_t server (gspc::net::serve ("tcp://localhost:*"));
  gspc::kvs::service_t service;
  gspc::net::handle ( "/service/kvs"
                    , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                       , boost::ref (service)
                                                       )
                    );

  std::cerr << "server running on: " << server->url () << std::endl;

  try
  {
    gspc::kvs::kvs_net_frontend_t kvs (server->url () + "?timeout=10000");

    std::vector<boost::shared_ptr<boost::thread> >
      threads;

    for (size_t i = 0 ; i < NTHREAD ; ++i)
    {
      threads.push_back
        (boost::shared_ptr<boost::thread>
        (new boost::thread (boost::bind ( &s_wfh_client_thread
                                        , i
                                        , &kvs
                                        , NUM
                                        , queue
                                        )
                           )
        ));
    }

    for (size_t i = 0 ; i < NTHREAD*NUM ; ++i)
    {
      rc = kvs.pop (queue, val, 1000);
      if (rc != 0)
      {
        std::cerr << "wfh: could not pop #" << i << " from '" << queue << "': "
                  << strerror (-rc)
                  << std::endl
          ;

        kvs.get (queue, val);
        std::cerr << "wfh: queue content: "
                  << pnet::type::value::show (val)
                  << std::endl
          ;

        break;
      }

      std::string from =
        boost::get<std::string>(*pnet::type::value::peek ("from", val));
      int msg =
        boost::get<int>(*pnet::type::value::peek ("msg", val));

      rc = kvs.push (from, msg);
      if (rc != 0)
      {
        std::cerr << "wfh: could not push #" << msg << " to '" << from << "': "
                  << strerror (-rc)
                  << std::endl
          ;
        break;
      }

      std::cerr << "wfh: sent reply " << i+1 << "/" << NTHREAD*NUM
                << " for request #" << msg
                << " to '" << from << "'"
                << std::endl
        ;
    }

    if (0 == rc)
    {
      std::cerr << "wfh: everything done" << std::endl;
    }
    else
    {
      std::cerr << "wfh: failed: " << strerror (-rc) << std::endl;
    }

    for (size_t i = 0 ; i < NTHREAD ; ++i)
    {
      threads [i]->join ();
    }
  }
  catch (std::exception const &ex)
  {
    throw;
  }

  BOOST_REQUIRE_EQUAL (rc, 0);
}
