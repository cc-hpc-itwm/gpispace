#define BOOST_TEST_MODULE MessageTest
#include <boost/test/unit_test.hpp>

#include <fhgcom/message.hpp>

#include <iostream>

BOOST_AUTO_TEST_CASE ( test_message_constr_default )
{
  fhg::com::message_t msg;
  BOOST_REQUIRE_EQUAL ( msg.size (), 0 );
}

BOOST_AUTO_TEST_CASE ( test_message_constr_with_length )
{
  fhg::com::message_t msg (10);
  BOOST_REQUIRE_EQUAL ( msg.size(), 10 );
}

BOOST_AUTO_TEST_CASE ( test_message_constr_with_data )
{
  const char * data = "Hello World!";
  fhg::com::message_t msg ( data, strlen(data)+1 );
  BOOST_REQUIRE_EQUAL ( msg.size(), (strlen(data)+1) );
}

BOOST_AUTO_TEST_CASE ( test_message_constr_copy )
{
  fhg::com::message_t msg1 ( "abc", 4 );
  fhg::com::message_t msg2 ( msg1 );
  BOOST_REQUIRE_EQUAL ( msg1.size(), 4 );
  BOOST_REQUIRE_EQUAL ( std::string(msg1.buf()), "abc" );

  BOOST_REQUIRE_EQUAL ( msg2.size(), 4 );
  BOOST_REQUIRE_EQUAL ( std::string(msg2.buf()), "abc" );
}

BOOST_AUTO_TEST_CASE ( test_message_assignment )
{
  fhg::com::message_t msg1;
  fhg::com::message_t msg2 ( 42 );
  msg1 = msg2;
  BOOST_REQUIRE_EQUAL ( msg1.size(), 42 );
}
