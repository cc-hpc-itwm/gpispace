#define BOOST_TEST_MODULE testDiscoverJobStates

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

static const boost::posix_time::milliseconds discover_interval (100);
static const int n_discover_ops(3);
