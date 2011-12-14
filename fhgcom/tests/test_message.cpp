#define BOOST_TEST_MODULE MessageTest
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fhgcom/message.hpp>
BOOST_AUTO_TEST_CASE ( test_message_constr_default )
{
  using namespace fhg::com;

  message_t msg;
  BOOST_REQUIRE ( msg.size () == 0 );
}

BOOST_AUTO_TEST_CASE ( test_message_constr_with_length )
{
  using namespace fhg::com;

  message_t msg (10);
  BOOST_REQUIRE ( msg.size() == 10 );
}

BOOST_AUTO_TEST_CASE ( test_message_constr_with_data )
{
  using namespace fhg::com;

  const char * data = "Hello World!";
  message_t msg ( data, strlen(data)+1 );
  BOOST_REQUIRE ( msg.size() == (strlen(data)+1) );
}

BOOST_AUTO_TEST_CASE ( test_message_constr_copy )
{
  using namespace fhg::com;
  message_t msg1 ( "abc", 4 );
  message_t msg2 ( msg1 );
  BOOST_REQUIRE ( msg1.size() == 4 );
  BOOST_REQUIRE ( std::string(msg1.buf()) == "abc" );

  BOOST_REQUIRE ( msg2.size() == 4 );
  BOOST_REQUIRE ( std::string(msg2.buf()) == "abc" );
}

BOOST_AUTO_TEST_CASE ( test_message_assignment )
{
  using namespace fhg::com;
  message_t msg1;
  message_t msg2 ( 42 );
  msg1 = msg2;
  BOOST_REQUIRE ( msg1.size() == 42 );
}
