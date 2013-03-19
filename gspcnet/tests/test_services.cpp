#define BOOST_TEST_MODULE GspcNetServices
#include <boost/test/unit_test.hpp>

#include <errno.h>

#include <cstring>
#include <iostream>
#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <gspc/net.hpp>
#include <gspc/net/error.hpp>
#include <gspc/net/handle.hpp>
#include <gspc/net/parse/parser.hpp>
#include <gspc/net/server/queue_manager.hpp>
#include <gspc/net/server/service_demux.hpp>

#include <gspc/net/handler/echo.hpp>
#include <gspc/net/handler/strip_prefix.hpp>

#include <gspc/net/frame_io.hpp>
#include <gspc/net/frame_util.hpp>

#include "mock_user.hpp"

BOOST_AUTO_TEST_CASE (test_echo_service)
{
  using namespace gspc::net::tests;

  mock::user user;
  int rc;

  gspc::net::server::service_demux_t demux;
  gspc::net::server::queue_manager_t qmgr (demux);

  demux.handle ("/test/echo", gspc::net::handler::echo ());

  gspc::net::frame rqst_frame;
  rqst_frame.set_command ("REQUEST");
  rqst_frame.set_header ("destination", "/test/echo");
  rqst_frame.set_header ("test-id", "42");
  rqst_frame.set_body ("Hello echo!");

  rc = qmgr.request (&user, "/test/echo", rqst_frame);
  BOOST_REQUIRE_EQUAL (rc, 0);

  BOOST_REQUIRE_EQUAL (user.frames.size (), 1);

  gspc::net::frame & rply_frame = user.frames.back ();

  BOOST_CHECK_EQUAL (rply_frame.get_command (), "REPLY");
  BOOST_CHECK_EQUAL (rply_frame.get_body_as_string (), "Hello echo!");
  BOOST_REQUIRE     (rply_frame.has_header ("test-id"));
  BOOST_CHECK_EQUAL (*rply_frame.get_header ("test-id"), "42");
}

BOOST_AUTO_TEST_CASE (test_strip_prefix)
{
  using namespace gspc::net::tests;

  mock::user user;
  int rc;

  gspc::net::server::service_demux_t demux;
  gspc::net::server::queue_manager_t qmgr (demux);

  demux.handle ( "/test/echo"
               , gspc::net::handler::strip_prefix ( "/test"
                                                  , gspc::net::handler::echo ()
                                                  )
               );

  gspc::net::frame rqst_frame;
  rqst_frame.set_command ("REQUEST");
  rqst_frame.set_header ("destination", "/test/echo");
  rqst_frame.set_header ("test-id", "42");
  rqst_frame.set_body ("Hello echo!");

  rc = qmgr.request (&user, "/test/echo", rqst_frame);
  BOOST_REQUIRE_EQUAL (rc, 0);

  BOOST_REQUIRE_EQUAL (user.frames.size (), 1);

  gspc::net::frame & rply_frame = user.frames.back ();

  BOOST_CHECK_EQUAL (rply_frame.get_command (), "REPLY");
  BOOST_CHECK_EQUAL (rply_frame.get_body_as_string (), "Hello echo!");
  BOOST_REQUIRE     (rply_frame.has_header ("test-id"));
  BOOST_CHECK_EQUAL (*rply_frame.get_header ("test-id"), "42");
}
