#define BOOST_TEST_MODULE GspcNetDeMux
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

#include <gspc/net/service/echo.hpp>

#include <gspc/net/frame_io.hpp>
#include <gspc/net/frame_util.hpp>

#include "mock_user.hpp"

static void s_echo_roundtrip ( gspc::net::server::queue_manager_t & qmgr
                             , gspc::net::tests::mock::user & user
                             , std::string const & destination
                             )
{
  int rc;

  gspc::net::frame rqst_frame;
  rqst_frame.set_command ("SEND");
  rqst_frame.set_header ("destination", destination);
  rqst_frame.set_header ("test-id", "42");
  rqst_frame.set_header ("reply-to", "/test/replies");
  rqst_frame.set_body ("Hello echo!");

  std::size_t old_frames_size = user.frames.size ();

  rc = qmgr.send ( &user
                 , *rqst_frame.get_header ("destination")
                 , rqst_frame
                 );
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (user.frames.size (), old_frames_size + 1);

  gspc::net::frame & rply_frame = user.frames.back ();

  BOOST_CHECK_EQUAL (rply_frame.get_command (), "MESSAGE");
  BOOST_CHECK_EQUAL (rply_frame.get_body (), "Hello echo!");
  BOOST_REQUIRE     (rply_frame.has_header ("test-id"));
  BOOST_CHECK_EQUAL (*rply_frame.get_header ("test-id"), "42");
}

BOOST_AUTO_TEST_CASE (test_multiple_echo_service)
{
  static const std::size_t NUM_REQUESTS = 1000;

  using namespace gspc::net::tests;

  mock::user user;

  gspc::net::server::service_demux_t demux;
  gspc::net::server::queue_manager_t qmgr (demux);

  qmgr.subscribe (&user, "/test/replies", "/test/replies", gspc::net::frame ());

  demux.handle ("/service/echo-1", gspc::net::service::echo ());
  demux.handle ("/service/echo-2", gspc::net::service::echo ());
  demux.handle ("/service/echo-3", gspc::net::service::echo ());

  for (std::size_t i = 0 ; i < NUM_REQUESTS ; ++i)
  {
    s_echo_roundtrip (qmgr, user, "/service/echo-1");
    s_echo_roundtrip (qmgr, user, "/service/echo-2");
    s_echo_roundtrip (qmgr, user, "/service/echo-3");
  }
}

struct erroneous_handler_t
{
  explicit
  erroneous_handler_t (std::string const &msg)
    : m_msg (msg)
  {}

  int operator() ( std::string const &
                 , gspc::net::frame const &
                 , gspc::net::user_ptr
                 )
  {
    throw std::runtime_error (m_msg);
  }
private:
  std::string m_msg;
};

BOOST_AUTO_TEST_CASE (test_erroneous_service)
{
  using namespace gspc::net::tests;

  int rc;
  mock::user user;

  gspc::net::server::service_demux_t demux;
  gspc::net::server::queue_manager_t qmgr (demux);

  qmgr.subscribe (&user, "/test/replies", "/test/replies", gspc::net::frame ());

  demux.handle ( "/service/error"
               , erroneous_handler_t
               ("erroneous_handler_t could not handle request")
               );

  gspc::net::frame rqst_frame;
  rqst_frame.set_command ("SEND");
  rqst_frame.set_header ("destination", "/service/error");
  rqst_frame.set_header ("test-id", "42");
  rqst_frame.set_header ("reply-to", "/test/replies");
  rqst_frame.set_body ("Hello echo!");

  rc = qmgr.send (&user, "/service/error", rqst_frame);
  BOOST_REQUIRE_EQUAL (rc, gspc::net::E_SERVICE_FAILED);

  BOOST_REQUIRE_EQUAL (user.frames.size (), 1);

  gspc::net::frame & rply_frame = user.frames.back ();

  BOOST_CHECK_EQUAL (rply_frame.get_command (), "ERROR");
  BOOST_CHECK       (rply_frame.has_header ("code"));
  BOOST_CHECK_EQUAL (*rply_frame.get_header ("code"), "503");
  BOOST_CHECK       (rply_frame.has_header ("message"));
  BOOST_CHECK_EQUAL ( *rply_frame.get_header ("message")
                    , "service failed"
                    );
}

BOOST_AUTO_TEST_CASE (test_no_such_service)
{
  using namespace gspc::net::tests;

  int rc;
  mock::user user;

  gspc::net::server::service_demux_t demux;
  gspc::net::server::queue_manager_t qmgr (demux);
  qmgr.subscribe (&user, "/test/replies", "/test/replies", gspc::net::frame ());

  gspc::net::frame rqst_frame;
  rqst_frame.set_command ("SEND");
  rqst_frame.set_header ("destination", "/service/echo");
  rqst_frame.set_header ("test-id", "42");
  rqst_frame.set_header ("reply-to", "/test/replies");
  rqst_frame.set_body ("Hello echo!");

  rc = qmgr.send (&user, "/service/error", rqst_frame);
  BOOST_REQUIRE_EQUAL (rc, gspc::net::E_SERVICE_LOOKUP);

  BOOST_REQUIRE_EQUAL (user.frames.size (), 1);

  gspc::net::frame & rply_frame = user.frames.back ();

  BOOST_CHECK_EQUAL (rply_frame.get_command (), "ERROR");
  BOOST_CHECK       (rply_frame.has_header ("code"));
  BOOST_CHECK_EQUAL (*rply_frame.get_header ("code"), "404");
  BOOST_CHECK       (rply_frame.has_header ("message"));
  BOOST_CHECK_EQUAL ( *rply_frame.get_header ("message")
                    , "no such service"
                    );
}

BOOST_AUTO_TEST_CASE (test_default_demux)
{
  using namespace gspc::net::tests;

  int rc;
  mock::user user;

  gspc::net::server::queue_manager_t qmgr;
  qmgr.subscribe (&user, "/test/replies", "/test/replies", gspc::net::frame ());

  gspc::net::handle ("/service/echo", gspc::net::service::echo ());

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
  BOOST_CHECK_EQUAL (rply_frame.get_body (), "Hello echo!");
  BOOST_REQUIRE     (rply_frame.has_header ("test-id"));
  BOOST_CHECK_EQUAL (*rply_frame.get_header ("test-id"), "42");
}

BOOST_AUTO_TEST_CASE (test_default_demux_multiple_mgmr)
{
  using namespace gspc::net::tests;

  int rc;
  mock::user user;

  gspc::net::server::queue_manager_t qmgr_1;
  qmgr_1.subscribe (&user, "/test/replies", "/test/replies", gspc::net::frame ());
  gspc::net::server::queue_manager_t qmgr_2;
  qmgr_2.subscribe (&user, "/test/replies", "/test/replies", gspc::net::frame ());

  gspc::net::handle ("/service/echo", gspc::net::service::echo ());

  gspc::net::frame rqst_frame;
  rqst_frame.set_command ("SEND");
  rqst_frame.set_header ("destination", "/service/echo");
  rqst_frame.set_header ("test-id", "42");
  rqst_frame.set_header ("reply-to", "/test/replies");
  rqst_frame.set_body ("Hello echo!");

  rc = qmgr_1.send (&user, "/service/echo", rqst_frame);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (user.frames.size (), 1);

  rc = qmgr_2.send (&user, "/service/echo", rqst_frame);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (user.frames.size (), 2);

  gspc::net::frame & rply_frame = user.frames.back ();

  BOOST_CHECK_EQUAL (rply_frame.get_command (), "MESSAGE");
  BOOST_CHECK_EQUAL (rply_frame.get_body (), "Hello echo!");
  BOOST_REQUIRE     (rply_frame.has_header ("test-id"));
  BOOST_CHECK_EQUAL (*rply_frame.get_header ("test-id"), "42");
}