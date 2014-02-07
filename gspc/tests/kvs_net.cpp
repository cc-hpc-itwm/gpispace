#define BOOST_TEST_MODULE GspcKvsNetTests
#include <boost/test/unit_test.hpp>

#include <gspc/kvs/api.hpp>
#include <gspc/kvs/impl/kvs_impl.hpp>
#include <gspc/kvs/impl/kvs_net_frontend.hpp>
#include <gspc/kvs/impl/kvs_net_service.hpp>
#include <gspc/net/server/queue_manager.hpp>
#include <gspc/net/server/service_demux.hpp>
#include <gspc/net/io.hpp>
#include <gspc/net/serve.hpp>
#include <gspc/net/server.hpp>
#include <gspc/net/service/strip_prefix.hpp>

#include <we/type/value/peek.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/now.hpp>

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>

namespace
{
  void push_value ( gspc::kvs::api_t *kvs
                  , gspc::kvs::api_t::key_type const &key
                  , gspc::kvs::api_t::value_type const &val
                  )
  {
    usleep (rand() % 1500);
    kvs->push (key, val);
  }
}

BOOST_AUTO_TEST_CASE (net_start_stop)
{
  gspc::net::initializer net_initializer;

  for (size_t i (0); i < 10; ++i)
  {
    gspc::net::server::service_demux_t service_demux;
    gspc::net::server::queue_manager_t queue_manager (service_demux);
    gspc::net::server_ptr_t server
      (gspc::net::serve ("tcp://localhost:*", queue_manager));
    gspc::kvs::service_t service;
    service_demux.handle ( "/service/kvs"
                      , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                         , boost::ref (service)
                                                         )
                      );

    gspc::kvs::kvs_net_frontend_t kvs (server->url(), net_initializer);

    BOOST_REQUIRE_EQUAL (kvs.put ("foo", std::string ("bar")), 0);
    gspc::kvs::api_t::value_type val;
    BOOST_REQUIRE_EQUAL (kvs.get ("foo", val), 0);
    BOOST_REQUIRE_EQUAL
      (val, pnet::type::value::value_type (std::string ("bar")));
  }
}

BOOST_AUTO_TEST_CASE (net_get_nokey)
{
  gspc::net::initializer net_initializer;

  gspc::kvs::api_t::value_type val;

  gspc::net::server::service_demux_t service_demux;
  gspc::net::server::queue_manager_t queue_manager (service_demux);
  gspc::net::server_ptr_t server (gspc::net::serve ("tcp://localhost:*", queue_manager));
  gspc::kvs::service_t service;
  service_demux.handle ( "/service/kvs"
                    , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                       , boost::ref (service)
                                                       )
                    );

  gspc::kvs::kvs_net_frontend_t kvs
    (server->url() + "?timeout=100", net_initializer);

  BOOST_REQUIRE_EQUAL (kvs.get ("foo", val), -ENOKEY);
}

BOOST_AUTO_TEST_CASE (net_api)
{
  gspc::net::initializer net_initializer;

  gspc::kvs::api_t::value_type val;

  gspc::net::server::service_demux_t service_demux;
  gspc::net::server::queue_manager_t queue_manager (service_demux);
  gspc::net::server_ptr_t server (gspc::net::serve ("tcp://localhost:*", queue_manager));
  gspc::kvs::service_t service;
  service_demux.handle ( "/service/kvs"
                    , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                       , boost::ref (service)
                                                       )
                    );

  gspc::kvs::kvs_net_frontend_t kvs
    (server->url() + "?timeout=100", net_initializer);

  BOOST_REQUIRE_EQUAL (kvs.get ("foo", val), -ENOKEY);
  BOOST_REQUIRE_EQUAL (kvs.put ("foo", "bar"), 0);
  BOOST_REQUIRE_EQUAL (kvs.get ("foo", val), 0);
  BOOST_REQUIRE_EQUAL (boost::get<std::string>(val), "bar");
  BOOST_REQUIRE_EQUAL (kvs.set_ttl ("foo", 60), 0);
  BOOST_REQUIRE_EQUAL (kvs.set_ttl_regex ("foo", 60), 0);
  BOOST_REQUIRE_EQUAL (kvs.push ("bar", "foo"), 0);
  BOOST_REQUIRE_EQUAL (kvs.pop ("bar", val), 0);
  BOOST_REQUIRE_EQUAL (boost::get<std::string>(val), "foo");
  BOOST_REQUIRE_EQUAL ( kvs.try_pop ("bar", val), -EAGAIN);
  BOOST_REQUIRE_EQUAL (kvs.try_pop ("bar", val), -EAGAIN);
  BOOST_REQUIRE_EQUAL (kvs.counter_reset ("cnt", 0), 0);
  {
    int cnt (0);
    BOOST_REQUIRE_EQUAL (kvs.counter_change ("cnt", cnt, +1), 0);
    BOOST_REQUIRE_EQUAL (cnt, 1);
    BOOST_REQUIRE_EQUAL (kvs.counter_change ("cnt", cnt, -1), 0);
    BOOST_REQUIRE_EQUAL (cnt, 0);
  }
  BOOST_REQUIRE_EQUAL (kvs.del_regex (".*"), 0);
  BOOST_REQUIRE_EQUAL (kvs.get ("bar", val), -ENOKEY);
  BOOST_REQUIRE_EQUAL (kvs.del ("cnt"), -ENOKEY);
  BOOST_REQUIRE_EQUAL (kvs.del ("foo"), -ENOKEY);
}

#ifdef NDEBUG
BOOST_AUTO_TEST_CASE (net_put_get)
{
  gspc::net::initializer net_initializer;

  gspc::kvs::api_t::value_type out;
  static gspc::kvs::api_t::value_type const in (std::string ("bar"));

  gspc::net::server::service_demux_t service_demux;
  gspc::net::server::queue_manager_t queue_manager (service_demux);
  gspc::net::server_ptr_t const server (gspc::net::serve ("tcp://localhost:*", queue_manager));
  gspc::kvs::service_t service;
  service_demux.handle ( "/service/kvs"
                    , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                       , boost::ref (service)
                                                       )
                    );

  gspc::kvs::kvs_net_frontend_t kvs
    (server->url () + "?timeout=100", net_initializer);

  static const size_t NUM (1 << 10);

  double duration (-fhg::util::now());
  for (size_t i (0); i < NUM; ++i)
  {
    BOOST_REQUIRE_EQUAL (kvs.put ("foo", in), 0);
    BOOST_REQUIRE_EQUAL (kvs.get ("foo", out), 0);
    BOOST_REQUIRE_EQUAL (in, out);
  }
  duration += fhg::util::now ();

  BOOST_REQUIRE_LT (duration, 2 * NUM / 4000.0);
}
#endif

BOOST_AUTO_TEST_CASE (net_wait)
{
  gspc::net::initializer net_initializer;

  gspc::kvs::api_t::value_type val;

  gspc::net::server::service_demux_t service_demux;
  gspc::net::server::queue_manager_t queue_manager (service_demux);
  gspc::net::server_ptr_t server (gspc::net::serve ("tcp://localhost:*", queue_manager));
  gspc::kvs::service_t service;
  service_demux.handle ( "/service/kvs"
                    , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                       , boost::ref (service)
                                                       )
                    );

  gspc::kvs::kvs_net_frontend_t kvs
    (server->url() + "?timeout=100", net_initializer);

  BOOST_REQUIRE_EQUAL
    (kvs.wait ("foo", gspc::kvs::api_t::E_EXIST, 500), -ETIME);

  gspc::kvs::api_t::value_type const val_to_push (std::string ("bar"));

  boost::thread pusher (boost::bind (&push_value, &kvs, "foo", val_to_push));

  BOOST_REQUIRE_NE
    (kvs.wait ("foo", gspc::kvs::api_t::E_PUSH, 2000) & gspc::kvs::api_t::E_PUSH, 0);

  if (pusher.joinable())
  {
    pusher.join();
  }

  BOOST_REQUIRE_EQUAL (kvs.pop ("foo", val), 0);
  BOOST_REQUIRE_EQUAL (val, val_to_push);
}

BOOST_AUTO_TEST_CASE (net_push_pop)
{
  gspc::net::initializer net_initializer;

  gspc::kvs::api_t::value_type val;

  gspc::net::server::service_demux_t service_demux;
  gspc::net::server::queue_manager_t queue_manager (service_demux);
  gspc::net::server_ptr_t server (gspc::net::serve ("tcp://localhost:*", queue_manager));
  gspc::kvs::service_t service;
  service_demux.handle ( "/service/kvs"
                    , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                       , boost::ref (service)
                                                       )
                    );

  gspc::kvs::kvs_net_frontend_t kvs
    (server->url() + "?timeout=100", net_initializer);

  gspc::kvs::api_t::value_type const val_to_push (std::string ("bar"));
  boost::thread pusher (boost::bind ( &push_value
                                    , &kvs
                                    , "foo"
                                    , val_to_push
                                    )
                       );

  BOOST_REQUIRE_EQUAL (kvs.pop ("foo", val, 1500), 0);
  BOOST_REQUIRE_EQUAL (val, val_to_push);

  if (pusher.joinable())
  {
    pusher.join();
  }
}

namespace
{
  void client_thread ( const size_t rank
                     , gspc::kvs::api_t *kvs
                     , const size_t nmsg
                     , const std::string &queue
                     )
  {
    const std::string my_queue ((boost::format ("thread-%1%") % rank).str());

    for (size_t i (0); i < nmsg; ++i)
    {
      pnet::type::value::value_type rply;

      BOOST_REQUIRE_EQUAL
        (kvs->push ( queue
                   , pnet::type::value::read
                     ((boost::format ( "Struct [from := \"%1%\", msg := %2%UL]"
                                     ) % my_queue % i
                      ).str ()
                     )
                   )
        , 0
        );
      BOOST_REQUIRE_EQUAL (kvs->pop (my_queue, rply, 10 * 1000), 0);
      BOOST_REQUIRE_EQUAL (rply, pnet::type::value::value_type (i));
    }
  }
}

BOOST_AUTO_TEST_CASE (net_many_push_pop)
{
  gspc::net::initializer net_initializer;

  gspc::kvs::api_t::value_type val;
  const std::string queue ("wfh");

  static const size_t NUM (10);
  static const size_t NTHREAD (15);

  gspc::net::server::service_demux_t service_demux;
  gspc::net::server::queue_manager_t queue_manager (service_demux);
  gspc::net::server_ptr_t server (gspc::net::serve ("tcp://localhost:*", queue_manager));
  gspc::kvs::service_t service;
  service_demux.handle ( "/service/kvs"
                    , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                       , boost::ref (service)
                                                       )
                    );

  gspc::kvs::kvs_net_frontend_t kvs
    (server->url () + "?timeout=100", net_initializer);

  boost::thread_group threads;

  for (size_t i (0); i < NTHREAD; ++i)
  {
    threads.create_thread (boost::bind (&client_thread, i, &kvs, NUM, queue));
  }

  for (size_t i (0); i < NTHREAD * NUM; ++i)
  {
    using pnet::type::value::peek;

    BOOST_REQUIRE_EQUAL (kvs.pop (queue, val, 1000), 0);
    BOOST_REQUIRE_EQUAL
      ( kvs.push ( boost::get<std::string> (*peek ("from", val))
                 , boost::get<size_t> (*peek ("msg", val))
                 )
      , 0
      );
  }

  threads.join_all();
}
