#define BOOST_TEST_MODULE GspcNetFrameBuilder
#include <boost/test/unit_test.hpp>

#include <cstring>
#include <iostream>
#include <algorithm>

#include <gspc/net/frame.hpp>
#include <gspc/net/frame_builder.hpp>

using namespace gspc::net;

BOOST_AUTO_TEST_CASE (test_error_frame)
{
  frame f = make::error_frame (E_INTERNAL_ERROR, "test error");

  BOOST_REQUIRE_EQUAL (f.get_command (), "ERROR");
  BOOST_REQUIRE_EQUAL (*f.get_header ("code"), "500");
  BOOST_REQUIRE_EQUAL (*f.get_header ("message"), "internal error");
  BOOST_REQUIRE_EQUAL (f.get_body_as_string (), "test error");
}

BOOST_AUTO_TEST_CASE (test_receipt_frame)
{
  frame f = make::receipt_frame
    (header::receipt ("42"));

  BOOST_REQUIRE_EQUAL (f.get_command (), "RECEIPT");
  BOOST_REQUIRE_EQUAL (*f.get_header ("receipt-id"), "42");
}

BOOST_AUTO_TEST_CASE (test_connected_frame)
{
  frame f = make::connected_frame (header::version ("1.0"));

  BOOST_REQUIRE_EQUAL (f.get_command (), "CONNECTED");
  BOOST_REQUIRE_EQUAL (*f.get_header ("version"), "1.0");
}

BOOST_AUTO_TEST_CASE (test_send_frame)
{
  frame f = make::send_frame
    (header::destination ("foo"));

  BOOST_REQUIRE_EQUAL (f.get_command (), "SEND");
  BOOST_REQUIRE_EQUAL (*f.get_header ("destination"), "foo");
}

BOOST_AUTO_TEST_CASE (test_message_frame)
{
  frame s = make::send_frame
    (header::destination ("foo"));
  s.set_header ("my-header-1", "1");
  s.set_header ("my-header-2", "2");
  s.set_header ("my-header-3", "3");

  frame f = make::message_frame (s);

  BOOST_REQUIRE_EQUAL (f.get_command (), "MESSAGE");
  BOOST_REQUIRE_EQUAL (*f.get_header ("destination"), "foo");
  BOOST_REQUIRE_EQUAL (*f.get_header ("my-header-1"), "1");
  BOOST_REQUIRE_EQUAL (*f.get_header ("my-header-2"), "2");
  BOOST_REQUIRE_EQUAL (*f.get_header ("my-header-3"), "3");
}

BOOST_AUTO_TEST_CASE (test_subscribe_frame)
{
  frame f = make::subscribe_frame
    ( header::destination ("foo")
    , header::id ("42")
    );

  BOOST_REQUIRE_EQUAL (f.get_command (), "SUBSCRIBE");
  BOOST_REQUIRE_EQUAL (*f.get_header ("destination"), "foo");
  BOOST_REQUIRE_EQUAL (*f.get_header ("id"), "42");
}

BOOST_AUTO_TEST_CASE (test_unsubscribe_frame)
{
  frame f = make::unsubscribe_frame
    (header::id ("42"));

  BOOST_REQUIRE_EQUAL (f.get_command (), "UNSUBSCRIBE");
  BOOST_REQUIRE_EQUAL (*f.get_header ("id"), "42");
}

BOOST_AUTO_TEST_CASE (test_ack_frame)
{
  frame f = make::ack_frame
    (header::id ("42"));

  BOOST_REQUIRE_EQUAL (f.get_command (), "ACK");
  BOOST_REQUIRE_EQUAL (*f.get_header ("id"), "42");
}

BOOST_AUTO_TEST_CASE (test_nack_frame)
{
  frame f = make::nack_frame
    (header::id ("42"));

  BOOST_REQUIRE_EQUAL (f.get_command (), "NACK");
  BOOST_REQUIRE_EQUAL (*f.get_header ("id"), "42");
}

BOOST_AUTO_TEST_CASE (test_begin_frame)
{
  frame f = make::begin_frame
    (header::transaction ("trans-1"));

  BOOST_REQUIRE_EQUAL (f.get_command (), "BEGIN");
  BOOST_REQUIRE_EQUAL (*f.get_header ("transaction"), "trans-1");
}

BOOST_AUTO_TEST_CASE (test_commit_frame)
{
  frame f = make::commit_frame
    (header::transaction ("trans-1"));

  BOOST_REQUIRE_EQUAL (f.get_command (), "COMMIT");
  BOOST_REQUIRE_EQUAL (*f.get_header ("transaction"), "trans-1");
}

BOOST_AUTO_TEST_CASE (test_abort_frame)
{
  frame f = make::abort_frame
    (header::transaction ("trans-1"));

  BOOST_REQUIRE_EQUAL (f.get_command (), "ABORT");
  BOOST_REQUIRE_EQUAL (*f.get_header ("transaction"), "trans-1");
}

BOOST_AUTO_TEST_CASE (test_connect_frame)
{
  frame f = make::connect_frame ();

  BOOST_REQUIRE_EQUAL (f.get_command (), "CONNECT");
}

BOOST_AUTO_TEST_CASE (test_disconnect_frame)
{
  frame f = make::disconnect_frame ();

  BOOST_REQUIRE_EQUAL (f.get_command (), "DISCONNECT");
}

BOOST_AUTO_TEST_CASE (test_heartbeat_frame)
{
  frame f = make::heartbeat_frame ();

  BOOST_REQUIRE_EQUAL (f.get_command (), "");
  BOOST_REQUIRE_EQUAL (f.get_header ().size (), 0u);
  BOOST_REQUIRE_EQUAL (f.get_body ().size (), 0u);
}
