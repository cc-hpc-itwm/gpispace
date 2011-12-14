#define BOOST_TEST_MODULE KvsMessageTest
#include <boost/test/unit_test.hpp>

#include <fhgcom/kvs/message/type.hpp>

#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

BOOST_AUTO_TEST_CASE( put_test )
{
  fhg::com::kvs::message::type m1
    (fhg::com::kvs::message::put("foo", "bar"));

  std::stringstream sstr;
  {
    boost::archive::text_oarchive ar(sstr);
    ar & m1;
  }

  fhg::com::kvs::message::type m2;
  {
    boost::archive::text_iarchive ar(sstr);
    ar & m2;
  }

  BOOST_REQUIRE( m1 == m2 );
}

BOOST_AUTO_TEST_CASE( get_test )
{
  fhg::com::kvs::message::type m1
    (fhg::com::kvs::message::msg_get("foo"));

  std::stringstream sstr;
  {
    boost::archive::text_oarchive ar(sstr);
    ar & m1;
  }

  fhg::com::kvs::message::type m2;
  {
    boost::archive::text_iarchive ar(sstr);
    ar & m2;
  }

  BOOST_REQUIRE( m1 == m2 );
}
