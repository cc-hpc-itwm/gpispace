#define BOOST_TEST_MODULE MessageTest
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fhgcom/message.hpp>
BOOST_AUTO_TEST_CASE ( test_message_constr_default )
{
  using namespace fhg::com;

  message_t msg;
  BOOST_REQUIRE ( msg.length() == 0 );
  BOOST_REQUIRE ( msg.data() == 0 );
}

BOOST_AUTO_TEST_CASE ( test_message_constr_with_length )
{
  using namespace fhg::com;

  message_t msg (10);
  BOOST_REQUIRE ( msg.length() == 10 );
  BOOST_REQUIRE ( msg.data() != 0 );
}

BOOST_AUTO_TEST_CASE ( test_message_constr_with_data )
{
  using namespace fhg::com;

  char * data = new char [42];
  char * ptr = data;
  message_t msg ( &data, 42 );
  BOOST_REQUIRE ( msg.length() == 42 );
  BOOST_REQUIRE ( msg.data() == ptr );
  BOOST_REQUIRE ( data == 0 );
}

BOOST_AUTO_TEST_CASE ( test_message_constr_copy )
{
  using namespace fhg::com;
  message_t msg1 ( 10 );
  message_t msg2 ( msg1 );
  BOOST_REQUIRE ( msg1.length() == 10 );
  BOOST_REQUIRE ( msg2.length() == 10 );
  BOOST_REQUIRE ( msg2.data () != 0 );
  BOOST_REQUIRE ( msg1.data () != msg2.data() );
}

BOOST_AUTO_TEST_CASE ( test_message_assignment )
{
  using namespace fhg::com;
  message_t msg1;
  message_t msg2 ( 42 );
  msg1 = msg2;
  BOOST_REQUIRE ( msg1.length() == 42 );
  BOOST_REQUIRE ( msg1.data () != 0 );
  BOOST_REQUIRE ( msg1.data () != msg2.data() );
}
