#define BOOST_TEST_MODULE GspcNetServices
#include <boost/test/unit_test.hpp>

#include <errno.h>

#include <cstring>
#include <iostream>
#include <algorithm>

#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <gspc/net.hpp>
#include <gspc/net/error.hpp>
#include <gspc/net/handle.hpp>
#include <gspc/net/parse/parser.hpp>
#include <gspc/net/server/queue_manager.hpp>
#include <gspc/net/server/service_demux.hpp>

#include <gspc/net/service/echo.hpp>
#include <gspc/net/service/strip_prefix.hpp>

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
  qmgr.subscribe (&user, "/test/replies", "/test/replies", gspc::net::frame ());

  demux.handle ("/service/echo", gspc::net::service::echo ());

  gspc::net::frame rqst_frame;
  rqst_frame.set_command ("SEND");
  rqst_frame.set_header ("destination", "/service/echo");
  rqst_frame.set_header ("test-id", "42");
  rqst_frame.set_header ("reply-to", "/test/replies");
  rqst_frame.set_body ("Hello echo!");

  rc = qmgr.send (&user, "/service/echo", rqst_frame);
  BOOST_REQUIRE_EQUAL (rc, 0);

  BOOST_REQUIRE_EQUAL (user.frames.size (), 1);

  gspc::net::frame & rply_frame = user.frames.back ();

  BOOST_CHECK_EQUAL (rply_frame.get_command (), "MESSAGE");
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
  qmgr.subscribe (&user, "/test/replies", "/test/replies", gspc::net::frame ());

  demux.handle ( "/service/echo"
               , gspc::net::service::strip_prefix ( "/service"
                                                  , gspc::net::service::echo ()
                                                  )
               );

  gspc::net::frame rqst_frame;
  rqst_frame.set_command ("SEND");
  rqst_frame.set_header ("destination", "/service/echo");
  rqst_frame.set_header ("test-id", "42");
  rqst_frame.set_header ("reply-to", "/test/replies");
  rqst_frame.set_body ("Hello echo!");

  rc = qmgr.send (&user, "/service/echo", rqst_frame);
  BOOST_REQUIRE_EQUAL (rc, 0);

  BOOST_REQUIRE_EQUAL (user.frames.size (), 1);

  gspc::net::frame & rply_frame = user.frames.back ();

  BOOST_CHECK_EQUAL (rply_frame.get_command (), "MESSAGE");
  BOOST_CHECK_EQUAL (rply_frame.get_body_as_string (), "Hello echo!");
  BOOST_REQUIRE     (rply_frame.has_header ("test-id"));
  BOOST_CHECK_EQUAL (*rply_frame.get_header ("test-id"), "42");
}

struct counting_service_t
{
  counting_service_t (const char * name)
    : counter (0)
    , name (name)
  {}

  void operator() ( std::string const &dst
                  , gspc::net::frame const &
                  , gspc::net::user_ptr
                  )
  {
    ++counter;
  }

  size_t                        counter;
  std::string name;
};

BOOST_AUTO_TEST_CASE (test_best_prefix_match)
{
  using namespace gspc::net::tests;

  int rc;
  mock::user user;
  gspc::net::frame rqst_frame;

  gspc::net::server::service_demux_t demux;

  counting_service_t service_A ("A");
  counting_service_t service_B ("B");

  demux.handle ("/echo/A", boost::ref (service_A));
  demux.handle ("/echo",   boost::ref (service_B));

  BOOST_REQUIRE_EQUAL (service_A.counter, 0);
  BOOST_REQUIRE_EQUAL (service_B.counter, 0);

  rc = demux.handle_request ("/echo", rqst_frame, &user);

  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service_A.counter, 0);
  BOOST_REQUIRE_EQUAL (service_B.counter, 1);

  rc = demux.handle_request ("/echo/A", rqst_frame, &user);

  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service_A.counter, 1);
  BOOST_REQUIRE_EQUAL (service_B.counter, 1);

  rc = demux.handle_request ("/echo/B", rqst_frame, &user);

  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service_A.counter, 1);
  BOOST_REQUIRE_EQUAL (service_B.counter, 2);

  rc = demux.handle_request ("/echo/BCD/EFG", rqst_frame, &user);

  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service_A.counter, 1);
  BOOST_REQUIRE_EQUAL (service_B.counter, 3);
}

BOOST_AUTO_TEST_CASE (test_best_prefix_match_no_separator)
{
  using namespace gspc::net::tests;

  int rc;
  mock::user user;
  gspc::net::frame rqst_frame;

  gspc::net::server::service_demux_t demux;

  counting_service_t service_A ("A");
  counting_service_t service_B ("B");

  demux.handle ("/echoA",  boost::ref (service_A));
  demux.handle ("/echoB",  boost::ref (service_B));

  BOOST_REQUIRE_EQUAL (service_A.counter, 0);
  BOOST_REQUIRE_EQUAL (service_B.counter, 0);

  rc = demux.handle_request ("/echo", rqst_frame, &user);

  BOOST_REQUIRE_EQUAL (rc, gspc::net::E_SERVICE_LOOKUP);
  BOOST_REQUIRE_EQUAL (service_A.counter, 0);
  BOOST_REQUIRE_EQUAL (service_B.counter, 0);

  rc = demux.handle_request ("/echoA", rqst_frame, &user);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service_A.counter, 1);
  BOOST_REQUIRE_EQUAL (service_B.counter, 0);

  rc = demux.handle_request ("/echoB", rqst_frame, &user);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service_A.counter, 1);
  BOOST_REQUIRE_EQUAL (service_B.counter, 1);

  rc = demux.handle_request ("/echoAB", rqst_frame, &user);
  BOOST_REQUIRE_EQUAL (rc, gspc::net::E_SERVICE_LOOKUP);
  BOOST_REQUIRE_EQUAL (service_A.counter, 1);
  BOOST_REQUIRE_EQUAL (service_B.counter, 1);
}

BOOST_AUTO_TEST_CASE (test_best_prefix_match_subservice)
{
  using namespace gspc::net::tests;

  int rc;
  mock::user user;
  gspc::net::frame rqst_frame;

  gspc::net::server::service_demux_t demux;

  counting_service_t service ("top");

  demux.handle ("/echo", boost::ref (service));

  BOOST_REQUIRE_EQUAL (service.counter, 0);

  rc = demux.handle_request ("/echo", rqst_frame, &user);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service.counter, 1);

  rc = demux.handle_request ("/echo/A", rqst_frame, &user);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service.counter, 2);

  rc = demux.handle_request ("/echo/B", rqst_frame, &user);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service.counter, 3);

  rc = demux.handle_request ("/echoA", rqst_frame, &user);
  BOOST_REQUIRE_EQUAL (rc, gspc::net::E_SERVICE_LOOKUP);
  BOOST_REQUIRE_EQUAL (service.counter, 3);
}

BOOST_AUTO_TEST_CASE (test_catch_all_service)
{
  using namespace gspc::net::tests;

  int rc;
  mock::user user;
  gspc::net::frame rqst_frame;

  gspc::net::server::service_demux_t demux;

  counting_service_t service ("catch-all");

  demux.handle ("/", boost::ref (service));

  BOOST_REQUIRE_EQUAL (service.counter, 0);

  rc = demux.handle_request ("/echo", rqst_frame, &user);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service.counter, 1);

  rc = demux.handle_request ("/echo/A", rqst_frame, &user);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service.counter, 2);

  rc = demux.handle_request ("/echo/B", rqst_frame, &user);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service.counter, 3);

  rc = demux.handle_request ("/echoA", rqst_frame, &user);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service.counter, 4);

  rc = demux.handle_request ("/", rqst_frame, &user);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (service.counter, 5);
}
