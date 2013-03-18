#define BOOST_TEST_MODULE GspcNetQueueManagerTest
#include <boost/test/unit_test.hpp>

#include <errno.h>

#include <cstring>
#include <iostream>
#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <gspc/net.hpp>
#include <gspc/net/parse/parser.hpp>
#include <gspc/net/server/queue_manager.hpp>
#include <gspc/net/frame_io.hpp>
#include <gspc/net/frame_util.hpp>

#include "mock_user.hpp"

BOOST_AUTO_TEST_CASE (test_subscribe_unsubscribe)
{
  using namespace gspc::net::tests;

  mock::user user;
  int rc;

  gspc::net::server::queue_manager_t qmgr;

  rc = qmgr.subscribe (&user, "/tests", "0");
  BOOST_CHECK_EQUAL (rc, 0);

  gspc::net::frame dummy;
  dummy.set_command ("SEND");
  dummy.set_header ("test-id", "42");

  rc = qmgr.send (&user, "/tests", dummy);
  BOOST_CHECK_EQUAL (rc, 0);

  BOOST_CHECK_EQUAL (user.frames.size (), 1);
  BOOST_CHECK_EQUAL (user.frames.back ().get_command (), "MESSAGE");
  BOOST_CHECK_EQUAL (*user.frames.back ().get_header ("test-id"), "42");

  user.frames.clear ();

  rc = qmgr.unsubscribe (&user, "0");
  rc = qmgr.send (&user, "/tests", dummy);
  BOOST_CHECK_EQUAL (rc, -ESRCH);
  BOOST_CHECK_EQUAL (user.frames.size (), 0);
}

BOOST_AUTO_TEST_CASE (test_subscribe_many_users)
{
  static const std::size_t NUM_MOCK_USERS = 5000;

  using namespace gspc::net::tests;

  std::vector<mock::user*> users;
  for (size_t i = 0 ; i < NUM_MOCK_USERS ; ++i)
  {
    users.push_back (new mock::user);
  }

  mock::user client;
  int rc;

  gspc::net::server::queue_manager_t qmgr;

  BOOST_FOREACH (mock::user *user, users)
  {
    rc = qmgr.subscribe (user, "/tests", "0");
    BOOST_CHECK_EQUAL (rc, 0);
  }

  gspc::net::frame dummy;
  dummy.set_command ("SEND");
  dummy.set_header ("test-id", "42");

  rc = qmgr.send (&client, "/tests", dummy);
  BOOST_CHECK_EQUAL (rc, 0);

  BOOST_FOREACH (mock::user *user, users)
  {
    BOOST_CHECK_EQUAL (user->frames.size (), 1);
    BOOST_CHECK_EQUAL (user->frames.back ().get_command (), "MESSAGE");
    BOOST_CHECK_EQUAL (*user->frames.back ().get_header ("test-id"), "42");
    user->frames.clear ();
  }

  BOOST_FOREACH (mock::user *user, users)
  {
    rc = qmgr.unsubscribe (user, "0");
    BOOST_CHECK_EQUAL (rc, 0);
    delete user;
  }
  users.clear ();
}

static void s_dummy_sender_thread ( gspc::net::server::queue_manager_t & qmgr
                                  , std::string const & dst
                                  , std::size_t num_to_send
                                  )
{
  using namespace gspc::net::tests;
  mock::user client;

  for (std::size_t i = 0 ; i < num_to_send ; ++i)
  {
    gspc::net::frame dummy;
    dummy.set_command ("SEND");
    gspc::net::frame_set_header (dummy, "test-id", i);

    qmgr.send (&client, dst, dummy);
  }
}

BOOST_AUTO_TEST_CASE (test_async_sender)
{
  static const std::size_t NUM_ASYNC_SENDER = 100;
  static const std::size_t NUM_MSGS_TO_SEND = 10000;

  using namespace gspc::net::tests;

  std::vector<boost::thread *> sender;
  mock::user client;
  int rc;

  gspc::net::server::queue_manager_t qmgr;
  rc = qmgr.subscribe (&client, "/tests", "sub-client-0");

  for (size_t i = 0 ; i < NUM_ASYNC_SENDER ; ++i)
  {
    sender.push_back (new boost::thread ( s_dummy_sender_thread
                                        , boost::ref (qmgr)
                                        , "/tests"
                                        , NUM_MSGS_TO_SEND
                                        )
                     );
  }

  BOOST_FOREACH (boost::thread *thrd, sender)
  {
    thrd->join ();
    delete thrd;
  }
  sender.clear ();

  BOOST_REQUIRE_EQUAL ( client.frames.size ()
                      , NUM_ASYNC_SENDER * NUM_MSGS_TO_SEND
                      );
}
