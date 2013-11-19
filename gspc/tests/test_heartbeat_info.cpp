#define BOOST_TEST_MODULE GspcNetHeartbeatInfo
#include <boost/test/unit_test.hpp>

#include <gspc/net/heartbeat_info.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

BOOST_AUTO_TEST_CASE (test_valid_heartbeat)
{
  using namespace gspc::net;
  {
    heartbeat_info_t hb;
    BOOST_REQUIRE_EQUAL (hb.recv_interval (), 0u);
    BOOST_REQUIRE_EQUAL (hb.send_interval (), 0u);
    BOOST_REQUIRE_EQUAL (hb.recv_duration (), boost::none);
    BOOST_REQUIRE_EQUAL (hb.send_duration (), boost::none);
  }

  {
    heartbeat_info_t hb ("0,0");
    BOOST_REQUIRE_EQUAL (hb.recv_interval (), 0u);
    BOOST_REQUIRE_EQUAL (hb.send_interval (), 0u);
    BOOST_REQUIRE_EQUAL (hb.recv_duration (), boost::none);
    BOOST_REQUIRE_EQUAL (hb.send_duration (), boost::none);
  }

  {
    heartbeat_info_t hb ("60,20");
    BOOST_REQUIRE_EQUAL (hb.recv_interval (), 60u);
    BOOST_REQUIRE_EQUAL (hb.send_interval (), 20u);
    BOOST_REQUIRE_EQUAL (*hb.recv_duration (), boost::posix_time::seconds (60));
    BOOST_REQUIRE_EQUAL (*hb.send_duration (), boost::posix_time::seconds (20));
  }
}

BOOST_AUTO_TEST_CASE (test_invalid_heartbeat)
{
  using namespace gspc::net;
  BOOST_REQUIRE_THROW (heartbeat_info_t ("a,b"), std::invalid_argument);
  BOOST_REQUIRE_THROW (heartbeat_info_t (" ,0"), std::invalid_argument);
  BOOST_REQUIRE_THROW (heartbeat_info_t (""), std::invalid_argument);
  BOOST_REQUIRE_THROW (heartbeat_info_t ("0,0 "), std::invalid_argument);
  BOOST_REQUIRE_THROW (heartbeat_info_t ("0,0,"), std::invalid_argument);
  BOOST_REQUIRE_THROW (heartbeat_info_t ("-1,0"), std::invalid_argument);
  BOOST_REQUIRE_THROW (heartbeat_info_t ("0,-1"), std::invalid_argument);
  BOOST_REQUIRE_THROW (heartbeat_info_t ("0"), std::invalid_argument);
  BOOST_REQUIRE_THROW (heartbeat_info_t (",0"), std::invalid_argument);
  BOOST_REQUIRE_THROW (heartbeat_info_t (",-1"), std::invalid_argument);
}
