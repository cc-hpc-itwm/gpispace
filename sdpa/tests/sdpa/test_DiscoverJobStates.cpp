#define BOOST_TEST_MODULE testDiscoverJobStates

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/peek.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

static const boost::posix_time::milliseconds discover_interval (100);
static const int n_discover_ops(3);

BOOST_AUTO_TEST_CASE (discover_discover_inexistent_job)
{
  using pnet::type::value::value_type;
  using pnet::type::value::peek;
  using pnet::type::value::show;

  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");

  sdpa::client::Client client (orchestrator.name());

  value_type disc_res(client.discoverJobStates("disc_id_0", "inexistent_job_id"));

  std::set<value_type> set_res(boost::get<std::set<value_type> >(disc_res));
  BOOST_FOREACH(const value_type& v, set_res)
  {
    BOOST_REQUIRE_EQUAL(boost::get<int>(peek("state", v).get()), sdpa::status::UNKNOWN);
  }
}

BOOST_AUTO_TEST_CASE (discover_one_orchestrator_no_agent)
{
  using pnet::type::value::value_type;
  using pnet::type::value::peek;
  using pnet::type::value::show;

  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id = client.submitJob (workflow);

  value_type disc_res;

  for(int i=0; i<n_discover_ops; i++)
  {
    std::ostringstream oss;
    oss<<"disc_id_"<<i;
    disc_res = (client.discoverJobStates(oss.str(), job_id));
    boost::this_thread::sleep (discover_interval);
  }

  // invariant: after some time, the leaf jobs are always in pending
  std::set<value_type> set_res(boost::get<std::set<value_type> >(disc_res));
  BOOST_FOREACH(const value_type& v, set_res)
  {
    BOOST_REQUIRE_EQUAL(boost::get<int>(peek("state", v).get()), sdpa::status::PENDING);
  }
}

BOOST_AUTO_TEST_CASE (discover_one_orchestrator_one_agent)
{
  using pnet::type::value::value_type;
  using pnet::type::value::peek;
  using pnet::type::value::show;

  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");

  const utils::agent agent
     ("agent_0", "127.0.0.1", orchestrator);

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id = client.submitJob (workflow);

  value_type disc_res;
  for(int i=0; i<n_discover_ops; i++)
  {
    std::ostringstream oss;
    oss<<"disc_id_"<<i;
    disc_res = (client.discoverJobStates(oss.str(), job_id));
    boost::this_thread::sleep (discover_interval);
  }

  // invariant: after some time, the leaf jobs are always in pending
  std::set<value_type> set_res(boost::get<std::set<value_type> >(disc_res));
  BOOST_FOREACH(const value_type& v, set_res)
  {
     BOOST_REQUIRE_EQUAL(boost::get<int>(peek("state", v).get()), sdpa::status::PENDING);
  }
}

BOOST_AUTO_TEST_CASE (insufficient_number_of_workers)
{
  using pnet::type::value::value_type;
  using pnet::type::value::peek;
  using pnet::type::value::show;

  const std::string workflow
    (utils::require_and_read_file ("workflows/stalled_workflow.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_A_0
    ( "drts_A_0", agent
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_A_1
    ( "drts_A_1", agent
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  const utils::drts_worker worker_B_0
    ( "drts_B_0", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_B_1
    ( "drts_B_1", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  // the task A requires 2 workers, task B requires 3 workers

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id = client.submitJob (workflow);

  value_type disc_res;
  for(int i=0; i<n_discover_ops; i++)
  {
      std::ostringstream oss;
      oss<<"disc_id_"<<i;
      disc_res = (client.discoverJobStates(oss.str(), job_id));
      boost::this_thread::sleep (discover_interval);
  }

  std::set<value_type> set_res(boost::get<std::set<value_type> >(disc_res));

  // invariant: after some time, the task B is always in pending
  bool b_at_least_one_job_in_pending(false);
  BOOST_FOREACH(const value_type& v, set_res)
  {
    if( boost::get<int>(peek("state", v).get()) == sdpa::status::PENDING )
    {
      b_at_least_one_job_in_pending = true;
      break;
    }
  }

  BOOST_REQUIRE(b_at_least_one_job_in_pending);
}
