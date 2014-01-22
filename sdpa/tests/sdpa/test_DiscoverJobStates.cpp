#define BOOST_TEST_MODULE testDiscoverJobStates

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

static const boost::posix_time::milliseconds discover_interval (100);
static const int n_discover_ops(3);

BOOST_AUTO_TEST_CASE (serialization_discovery_info_1)
{
  std::stringstream sstr;
  boost::archive::text_oarchive oar (sstr);
  sdpa::discovery_info_t disc_info("job_0", boost::none, sdpa::discovery_info_set_t());

  oar << disc_info;

  sdpa::discovery_info_t restored_disc_info;
  std::istringstream isstr (sstr.str());
  boost::archive::text_iarchive iar (sstr);

  iar >> restored_disc_info;
  BOOST_REQUIRE(disc_info == restored_disc_info);
}

BOOST_AUTO_TEST_CASE (serialization_discovery_info_2)
{
  std::stringstream sstr;
  boost::archive::text_oarchive oar (sstr);
  sdpa::discovery_info_t disc_info_child_1("job_0_1", sdpa::status::PENDING, sdpa::discovery_info_set_t());
  sdpa::discovery_info_t disc_info_child_2("job_0_2", sdpa::status::FINISHED, sdpa::discovery_info_set_t());
  sdpa::discovery_info_t disc_info_child_3("job_0_3", sdpa::status::FAILED, sdpa::discovery_info_set_t());

  sdpa::discovery_info_set_t disc_info_set;
  disc_info_set.insert(disc_info_child_1);
  disc_info_set.insert(disc_info_child_2);
  disc_info_set.insert(disc_info_child_3);

  sdpa::discovery_info_t disc_info("job_0", boost::none, disc_info_set);

  oar << disc_info;

  sdpa::discovery_info_t restored_disc_info;
  std::istringstream isstr (sstr.str());
  boost::archive::text_iarchive iar (sstr);

  iar >> restored_disc_info;
  BOOST_REQUIRE(disc_info == restored_disc_info);
}
