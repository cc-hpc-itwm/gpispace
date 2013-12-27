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
  BOOST_REQUIRE_EQUAL (f.get_body (), "test error");
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
    (header::destination ("foo"), "", 0);

  BOOST_REQUIRE_EQUAL (f.get_command (), "SEND");
  BOOST_REQUIRE_EQUAL (*f.get_header ("destination"), "foo");
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
