#define BOOST_TEST_MODULE testDiscoverJobStates

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <we/layer.hpp>

#include <boost/test/unit_test.hpp>
#include <sdpa/types.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

static const boost::posix_time::milliseconds discover_interval (100);
static const int n_discover_ops(3);

boost::mutex generate_id_mutex;
we::layer::id_type generate_id()
{
 boost::mutex::scoped_lock const _ (generate_id_mutex);
 static unsigned long _cnt (0);
 return boost::lexical_cast<we::layer::id_type> (++_cnt);
}

namespace sdpa {
  namespace daemon {
    class TestAgent : public Agent
    {
    public:
      TestAgent( const std::string& name
                , const std::string& url
                , const sdpa::master_info_list_t arrMasterNames
                , int rank )
        : Agent (name, url, arrMasterNames, rank, boost::none)
      {
        delete ptr_workflow_engine_;
        ptr_workflow_engine_ =  new we::layer
            ( boost::bind (&TestAgent::submit, this, _1, _2)
            , boost::bind (&TestAgent::cancel, this, _1)
            , boost::bind (&TestAgent::finished, this, _1, _2)
            , boost::bind (&TestAgent::failed, this, _1, _2, _3)
            , boost::bind (&TestAgent::canceled, this, _1)
            , boost::bind (&TestAgent::discover, this, _1, _2)
            , boost::bind (&TestAgent::discovered, this, _1, _2)
            , boost::bind (&::generate_id)
            , *_random_extraction_engine
            );
      }

      int n_child_jobs() { return _list_child_jobs.size();}
      std::string gen_id() { return GenericDaemon::gen_id(); }
      void discover (we::layer::id_type discover_id, we::layer::id_type job_id)
      {
        BOOST_REQUIRE(std::find(_list_child_jobs.begin(), _list_child_jobs.end(), job_id) != _list_child_jobs.end());
      }

      void discovered (we::layer::id_type discover_id, sdpa::discovery_info_t discover_result)
      {
        BOOST_FOREACH(const sdpa::discovery_info_t& child_info, discover_result.children())
        {
           BOOST_REQUIRE_EQUAL(child_info.state(), sdpa::status::PENDING);
        }
      }

      void submit( const we::layer::id_type& activityId, const we::type::activity_t& activity )
      {
        _list_child_jobs.push_back(activityId);

        if(_list_child_jobs.size()==2)
          _cond_wait_submit.notify_all();
      }

      void wait()
      {
        boost::mutex::scoped_lock lock(_mtx_submit);
        _cond_wait_submit.wait(lock);
      }

      void notify_discovered_activities(we::layer::id_type discover_id)
      {
        BOOST_FOREACH(const sdpa::job_id_t& job_id, _list_child_jobs)
        {
          sdpa::discovery_info_t discover_result(job_id, sdpa::status::PENDING, sdpa::discovery_info_set_t());
          workflowEngine()->discovered(discover_id, discover_result);
        }
      }

    private:
      std::list< we::layer::id_type> _list_child_jobs;
      mutable boost::mutex _mtx_submit;
      boost::condition_variable_any _cond_wait_submit;
    };
}}

BOOST_AUTO_TEST_CASE(test_activity_1)
{
  const we::layer::id_type discovery_id("disc_id_0");
  const std::string workflow
     (utils::require_and_read_file ("workflows/coallocation_test2.pnet"));

  const we::type::activity_t activity (workflow);

  sdpa::master_info_list_t listMasterInfo;
  sdpa::daemon::TestAgent agent("agent_0", "127.0.0.1", listMasterInfo, 0);

  we::layer::id_type const id (generate_id());

  agent.workflowEngine()->submit (id, activity);

  agent.wait();

  agent.workflowEngine()->discover (discovery_id, id);
  agent. notify_discovered_activities(discovery_id);
}

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

BOOST_AUTO_TEST_CASE (discover_discover_inexistent_job)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");

  sdpa::client::Client client (orchestrator.name());

  sdpa::discovery_info_t disc_res(client.discoverJobStates("disc_id_0", "inexistent_job_id"));

  BOOST_REQUIRE_EQUAL(disc_res.state(), boost::none);
}

BOOST_AUTO_TEST_CASE (discover_one_orchestrator_no_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id = client.submitJob (workflow);

  sdpa::discovery_info_t disc_res;

  for(int i=0; i<n_discover_ops; i++)
  {
    std::ostringstream oss;
    oss<<"disc_id_"<<i;
    disc_res = (client.discoverJobStates(oss.str(), job_id));
    boost::this_thread::sleep (discover_interval);
  }

  BOOST_REQUIRE_EQUAL(disc_res.state(), sdpa::status::PENDING);
}

BOOST_AUTO_TEST_CASE (discover_one_orchestrator_one_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");

  const utils::agent agent
     ("agent_0", "127.0.0.1", orchestrator);

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id = client.submitJob (workflow);

  sdpa::discovery_info_t disc_res;
  for(int i=0; i<n_discover_ops; i++)
  {
    std::ostringstream oss;
    oss<<"disc_id_"<<i;
    disc_res = (client.discoverJobStates(oss.str(), job_id));
    boost::this_thread::sleep (discover_interval);
  }

  // invariant: after some time, the leaf jobs are always in pending
  BOOST_FOREACH(const sdpa::discovery_info_t& child_info, disc_res.children())
  {
     BOOST_REQUIRE_EQUAL(child_info.state(), sdpa::status::PENDING);
  }
}

BOOST_AUTO_TEST_CASE (insufficient_number_of_workers)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test2.pnet"));

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

  sdpa::discovery_info_t disc_res;
  for(int i=0; i<n_discover_ops; i++)
  {
      std::ostringstream oss;
      oss<<"disc_id_"<<i;
      disc_res = (client.discoverJobStates(oss.str(), job_id));
      boost::this_thread::sleep (discover_interval);
  }

  // invariant: after some time, the task B is always in pending
  BOOST_FOREACH(const sdpa::discovery_info_t& child_info, disc_res.children())
  {
    BOOST_REQUIRE( child_info.state() == sdpa::status::PENDING || child_info.state() == sdpa::status::FINISHED );
  }

  bool b_at_least_one_job_in_pending(false);
  BOOST_FOREACH(const sdpa::discovery_info_t& child_info, disc_res.children())
  {
    if(child_info.state() == sdpa::status::PENDING)
    {
      b_at_least_one_job_in_pending = true;
      break;
    }
  }

  BOOST_REQUIRE(b_at_least_one_job_in_pending);
}
