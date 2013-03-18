#define BOOST_TEST_MODULE GspcNetQueueManagerTest
#include <boost/test/unit_test.hpp>

#include <errno.h>

#include <cstring>
#include <iostream>
#include <algorithm>

#include <boost/foreach.hpp>

#include <gspc/net.hpp>
#include <gspc/net/parse/parser.hpp>
#include <gspc/net/server/queue_manager.hpp>
#include <gspc/net/frame_io.hpp>

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
