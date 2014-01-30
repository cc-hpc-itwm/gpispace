#define BOOST_TEST_MODULE testDiscoverJobStates

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <we/layer.hpp>

#include <boost/test/unit_test.hpp>
#include <sdpa/types.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

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
      typedef std::pair<we::layer::id_type, sdpa::discovery_info_t> pair_discovery_result_t;
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

      std::string gen_id() { return GenericDaemon::gen_id(); }
      void discover (we::layer::id_type discover_id, we::layer::id_type job_id)
      {
        boost::unique_lock<boost::mutex> const _ (_mtx_disc);
        _jobs_to_discover.push_back(std::make_pair(discover_id, job_id));
        _cond_disc.notify_all();
      }

      void discovered (we::layer::id_type discover_id, sdpa::discovery_info_t discover_result)
      {
        boost::unique_lock<boost::mutex> lock_res(_mtx_result);
        _discovery_result = discover_result;
        _cond_result.notify_one();
      }

      void submit( const we::layer::id_type& activityId, const we::type::activity_t& activity                         )
      {
        GenericDaemon::submit(activityId, activity);
        if(jobManager().numberOfJobs()<2)
          _cond_all_submitted.notify_one();
      }

      void wait_all_submitted()
      {
        boost::unique_lock<boost::mutex> const _(_mtx_all_submitted);
        _cond_all_submitted.wait(_mtx_all_submitted);
      }

      void notify_discovered()
      {
        while(true)
        {
          boost::unique_lock<boost::mutex> lock(_mtx_disc);
          _cond_disc.timed_wait(lock, boost::posix_time::seconds(1));

          if(!_jobs_to_discover.empty())
          {
            std::pair<we::layer::id_type, sdpa::job_id_t> pair = _jobs_to_discover.front();
            _jobs_to_discover.pop_front();

            Job* pJob = jobManager().findJob(pair.second);
            sdpa::discovery_info_t discover_result(pair.second, pJob?boost::optional<sdpa::status::code>(pJob->getStatus()):boost::none, sdpa::discovery_info_set_t());
            workflowEngine()->discovered(pair.first, discover_result);
          }
        }
      }

      void wait_for_discovery_result()
      {
        boost::unique_lock<boost::mutex> lock_res(_mtx_result);
        _cond_result.wait(lock_res);
      }

      bool test_invariant()
      {
        if(_discovery_result.children().size()<2)
          return false;

        bool b_invariant(true);
        BOOST_FOREACH(const sdpa::discovery_info_t& child_info, _discovery_result.children())
        {
          if(!child_info.state() || child_info.state().get() != sdpa::status::PENDING)
          {
              b_invariant = false;
              break;
          }
        }

        return b_invariant;
      }

    private:
      boost::mutex _mtx_disc;
      boost::mutex _mtx_result;
      boost::mutex _mtx_all_submitted;
      boost::condition_variable_any _cond_disc;
      boost::condition_variable_any _cond_result;
      boost::condition_variable_any _cond_all_submitted;
      std::deque<std::pair<we::layer::id_type, sdpa::job_id_t> > _jobs_to_discover;
      sdpa::discovery_info_t _discovery_result;
    };
}}

BOOST_AUTO_TEST_CASE(test_discover_activities)
{

  const std::string workflow
     (utils::require_and_read_file ("workflows/coallocation_test2.pnet"));

  const we::type::activity_t activity (workflow);

  sdpa::master_info_list_t listMasterInfo;
  sdpa::daemon::TestAgent agent("agent_0", "127.0.0.1", listMasterInfo, 0);

  we::layer::id_type const id (generate_id());

  agent.workflowEngine()->submit (id, activity);
  //! Note it seems to have a race condition somewhere if we call layer submit and
  // layer discover immediately (deadlock)
  agent.wait_all_submitted();

  boost::thread thrd_notify(boost::thread(&sdpa::daemon::TestAgent::notify_discovered, &agent));

  int i=0;
  while(!agent.test_invariant())
  {
      std::ostringstream oss;
      oss<<"disc_id_"<<i++;
      agent.workflowEngine()->discover (oss.str(), id);
      agent.wait_for_discovery_result();
  }

  thrd_notify.interrupt();
  if(thrd_notify.joinable())
    thrd_notify.join();
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

  int i=0;
  bool b_invariant(false);
  while(!b_invariant)
  {
    std::ostringstream oss;
    oss<<"disc_id_"<<i++;
    disc_res = (client.discoverJobStates(oss.str(), job_id));

    b_invariant = (disc_res.state() && disc_res.state().get() == sdpa::status::PENDING);
  }
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

  int i=0;
  sdpa::discovery_info_t disc_res;
  bool b_invariant(false);
  while(!b_invariant)
  {
    std::ostringstream oss;
    oss<<"disc_id_"<<i++;
    disc_res = (client.discoverJobStates(oss.str(), job_id));

    // invariant: after some time, the leaf jobs are always in pending
    b_invariant = true;
    BOOST_FOREACH(const sdpa::discovery_info_t& child_info, disc_res.children())
    {
      if(!child_info.state() || child_info.state() != sdpa::status::PENDING)
      {
          b_invariant = false;
          break;
      }
    }
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

  int i=0;
  sdpa::discovery_info_t disc_res;

  // invariant: after some time, all tasks are pending
  bool b_all_pending(false);
  do
  {
    std::ostringstream oss;
    oss<<"disc_id_"<<i++;
    disc_res = client.discoverJobStates(oss.str(), job_id);
    b_all_pending = true;
    BOOST_FOREACH(const sdpa::discovery_info_t& child_info, disc_res.children())
    {
      if(!child_info.state() || (child_info.state() && child_info.state().get() != sdpa::status::PENDING) )
      {
        b_all_pending = false;
        break;
      }
    }
  } while(!b_all_pending || disc_res.children().size()!=2);
}

BOOST_AUTO_TEST_CASE (remove_workers)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

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

  boost::scoped_ptr<utils::drts_worker> ptr_worker_A_1
   (  new utils::drts_worker
       ( "drts_A_1", agent
       , "A"
       , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
       , kvs_host(), kvs_port()
       )
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

  boost::scoped_ptr<utils::drts_worker> ptr_worker_B_2
    (  new utils::drts_worker
        ( "drts_B_2", agent
        , "B"
        , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
        , kvs_host(), kvs_port()
        )
    );

  // the task A requires 2 workers, task B requires 3 workers

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id = client.submitJob (workflow);

  ptr_worker_A_1.reset();
  ptr_worker_B_2.reset();

  int i=0;
  sdpa::discovery_info_t disc_res;

  // invariant: after some time, all tasks are pending
  bool b_all_pending(false);
  do
  {
     std::ostringstream oss;
     oss<<"disc_id_"<<i++;
     disc_res = client.discoverJobStates(oss.str(), job_id);
     b_all_pending = true;
     BOOST_FOREACH(const sdpa::discovery_info_t& child_info, disc_res.children())
     {
       if(!child_info.state() ||  child_info.state().get() != sdpa::status::PENDING)
       {
         b_all_pending = false;
         break;
       }
     }
  } while(!b_all_pending || disc_res.children().empty());
}
