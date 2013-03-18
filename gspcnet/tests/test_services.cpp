#define BOOST_TEST_MODULE GspcNetServices.cpp
#include <boost/test/unit_test.hpp>

#include <errno.h>

#include <cstring>
#include <iostream>
#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <gspc/net.hpp>
#include <gspc/net/error.hpp>
#include <gspc/net/parse/parser.hpp>
#include <gspc/net/server/queue_manager.hpp>
#include <gspc/net/server/service_demux.hpp>
#include <gspc/net/handler/echo.hpp>

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

static void s_echo_roundtrip ( gspc::net::server::queue_manager_t & qmgr
                             , gspc::net::tests::mock::user & user
                             , std::string const & destination
                             )
{
  int rc;

  gspc::net::frame rqst_frame;
  rqst_frame.set_command ("REQUEST");
  rqst_frame.set_header ("destination", destination);
  rqst_frame.set_header ("test-id", "42");
  rqst_frame.set_body ("Hello echo!");

  std::size_t old_frames_size = user.frames.size ();

  rc = qmgr.request ( &user
                    , *rqst_frame.get_header ("destination")
                    , rqst_frame
                    );
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (user.frames.size (), old_frames_size + 1);

  gspc::net::frame & rply_frame = user.frames.back ();

  BOOST_CHECK_EQUAL (rply_frame.get_command (), "REPLY");
  BOOST_CHECK_EQUAL (rply_frame.get_body_as_string (), "Hello echo!");
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

  demux.handle ("/test/echo-1", gspc::net::handler::echo ());
  demux.handle ("/test/echo-2", gspc::net::handler::echo ());
  demux.handle ("/test/echo-3", gspc::net::handler::echo ());

  for (std::size_t i = 0 ; i < NUM_REQUESTS ; ++i)
  {
    s_echo_roundtrip (qmgr, user, "/test/echo-1");
    s_echo_roundtrip (qmgr, user, "/test/echo-2");
    s_echo_roundtrip (qmgr, user, "/test/echo-3");
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
                 , gspc::net::frame &
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

  demux.handle ( "/tests/error"
               , erroneous_handler_t
                 ("erroneous_handler_t could not handle request")
               );

  gspc::net::frame rqst_frame;
  rqst_frame.set_command ("REQUEST");
  rqst_frame.set_header ("destination", "/tests/error");
  rqst_frame.set_header ("test-id", "42");
  rqst_frame.set_body ("Hello echo!");

  rc = qmgr.request (&user, "/tests/error", rqst_frame);
  BOOST_REQUIRE_EQUAL (rc, gspc::net::E_INTERNAL_ERROR);

  BOOST_REQUIRE_EQUAL (user.frames.size (), 1);

  gspc::net::frame & rply_frame = user.frames.back ();

  BOOST_CHECK_EQUAL (rply_frame.get_command (), "ERROR");
  BOOST_CHECK       (rply_frame.has_header ("error-code"));
  BOOST_CHECK_EQUAL (*rply_frame.get_header ("error-code"), "500");
  BOOST_CHECK       (rply_frame.has_header ("error-message"));
  BOOST_CHECK_EQUAL ( *rply_frame.get_header ("error-message")
                    , "internal error"
                    );
  BOOST_CHECK_EQUAL ( rply_frame.get_body_as_string ()
                    , "erroneous_handler_t could not handle request"
                    );
}
