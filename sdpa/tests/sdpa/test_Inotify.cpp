#define BOOST_TEST_MODULE testInotify

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

namespace
{
  void touch_file_until_flag_set (bool* keep_on, std::string filename)
  {
    while (*keep_on)
    {
      std::ifstream ifs (filename.c_str());
      if(!ifs.good())
      {
        std::ofstream ofs (filename.c_str());
      }
      boost::this_thread::sleep (boost::posix_time::milliseconds (50));
    }
  }
}

BOOST_AUTO_TEST_CASE (testInotifyExecution)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/inotify.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent<we::mgmt::layer> agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_0
    ( "drts_0", agent
    , "ATOMIC"
    , TESTS_EXAMPLE_INOTIFY_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  bool keep_on_touching (true);
  boost::thread toucher ( boost::bind ( &touch_file_until_flag_set
                                      , &keep_on_touching
                                      , "inotify_test.txt"
                                      )
                        );

  utils::client::submit_job_and_wait_for_termination
    (workflow, "sdpac", orchestrator);

  keep_on_touching = false;
  if (toucher.joinable())
  {
    toucher.join();
  }
}
