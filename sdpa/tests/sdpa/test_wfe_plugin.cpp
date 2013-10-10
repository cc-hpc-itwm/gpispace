#define BOOST_TEST_MODULE TestWfePlugin
#include <boost/test/unit_test.hpp>

#include "tests_config.hpp"
#include "kvs_setup_fixture.hpp"

#include <boost/thread.hpp>

//plugin
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>

#include <plugins/kvs.hpp>
#include <plugins/wfe.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup);

static int s_load_unload_test ()
{
  static const int NUM_ITERATIONS = 50;

  int rc = 0;

  fhg::core::kernel_t *kernel = new fhg::core::kernel_t;
  kernel->set_name ("wfe_plugin_test");
  boost::thread kernel_thread =
    boost::thread (&fhg::core::kernel_t::run, kernel);
  kernel->put ("plugin.kvs.host", kvs_host ());
  kernel->put ("plugin.kvs.port", kvs_port ());
  kernel->put ("plugin.wfe.library_path", TESTS_EXAMPLE_ATOMIC_MODULES_PATH);

  for (int i = 0 ; i < NUM_ITERATIONS && rc == 0 ; ++i)
  {
    kernel->load_plugin (TESTS_KVS_PLUGIN_PATH);
    kernel->load_plugin (TESTS_WFE_PLUGIN_PATH);

    kvs::KeyValueStore *kvs_p =
      kernel->lookup_plugin_as<kvs::KeyValueStore>("kvs");

    BOOST_REQUIRE (kvs_p);

    kvs_p->put ("test.i", i);

    wfe::WFE *wfe =
      kernel->lookup_plugin_as<wfe::WFE>("wfe");

    BOOST_REQUIRE (wfe);

    std::stringstream jobdesc;
    std::ifstream ifs (TESTS_WORKFLOWS_PATH "/atomic_wfe_plugin_test.pnet");
    ifs >> std::noskipws >> jobdesc.rdbuf ();

    std::string result;
    std::string error_message;

    unlink ("atomic_wfe_plugin_test.txt");

    std::list<std::string> worker_list;
    worker_list.push_back ("worker-localhost-1");

    rc = wfe->execute ("test_job"
                      , jobdesc.str ()
                      , wfe::capabilities_t ()
                      , result
                      , error_message
                      , worker_list
                      );

    {
      std::size_t counter_value;
      std::ifstream ifs ("atomic_wfe_plugin_test.txt");

      BOOST_REQUIRE (ifs.good ());

      ifs >> counter_value;

      BOOST_CHECK_EQUAL (counter_value, 20u);
    }

    unlink ("atomic_wfe_plugin_test.txt");

    kernel->unload_all ();

    BOOST_CHECK_EQUAL (rc, 0);
  }

  kernel->stop ();
  kernel_thread.join ();
  delete kernel;

  return rc;
}

BOOST_AUTO_TEST_CASE (test_load_unload_plugins)
{
  setenv ("FHGLOG_level", "WARN", 1);
  FHGLOG_SETUP ();

  static const int NUM_ITERATIONS = 10;

  for (int i = 0 ; i < NUM_ITERATIONS ; ++i)
  {
    int rc = s_load_unload_test ();
    BOOST_CHECK_EQUAL (rc, 0);
  }
}
