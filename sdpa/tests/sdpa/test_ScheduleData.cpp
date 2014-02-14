#define BOOST_TEST_MODULE testScheduleData

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>
#include <we/layer.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

namespace sdpa
{
  namespace daemon
  {
    class TestAgent : public Agent
    {
    public:
      TestAgent ( const std::string& name
                , const std::string& url
                )
        : Agent (name, url,  kvs_host(), kvs_port(), sdpa::master_info_list_t(), boost::none)
          , _n_submitted_activities(0)
      {}

      void submit( const we::layer::id_type&, const we::type::activity_t& activity)
      {
        boost::optional<unsigned long>
          num_workers( activity.transition().get_schedule_data<unsigned long> (activity.input(), "num_worker")) ;

        if(!num_workers )
        {
          throw std::runtime_error("The number of workers is not set");
        }

        _set_num_workers_req.insert (num_workers.get());

        boost::unique_lock<boost::mutex> const _ (_mtx_all_submitted);
        _n_submitted_activities++;
        _cond_all_submitted.notify_one();
      }

      void wait_all_submitted()
      {
        boost::unique_lock<boost::mutex> const _ (_mtx_all_submitted);
        while (_n_submitted_activities < 2)
          _cond_all_submitted.wait (_mtx_all_submitted);
      }

      std::set<unsigned long> set_num_workers_req()
      {
        return _set_num_workers_req;
      }

    private:

      boost::mutex _mtx_all_submitted;
      boost::condition_variable_any _cond_all_submitted;
      unsigned long _n_submitted_activities;
      std::set<unsigned long> _set_num_workers_req;
    };
  }
}

BOOST_AUTO_TEST_CASE (num_workers_required_is_0)
{
  // this workflow produces two activities, each requiring 0 workers
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_bad_test.pnet"));

  const we::type::activity_t activity (workflow);

  sdpa::daemon::TestAgent agent ("agent_0", "127.0.0.1");

  agent.workflowEngine()->submit ("test_job", activity);
  agent.wait_all_submitted();

  std::set<unsigned long> expected_set;
  expected_set.insert(0);
  BOOST_REQUIRE( agent.set_num_workers_req() == expected_set);
}

BOOST_AUTO_TEST_CASE (valid_num_workers_required)
{
  // this workflow produces two activities, each requiring 0 workers
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test2.pnet"));

  const we::type::activity_t activity (workflow);

  sdpa::master_info_list_t listMasterInfo;
  sdpa::daemon::TestAgent agent ("agent_0", "127.0.0.1");

  agent.workflowEngine()->submit ("test_job", activity);
  agent.wait_all_submitted();

  std::set<unsigned long> expected_set;
  // task A expected to require 2 workers
  expected_set.insert(2ul);
  // task A expected to require 3 workers
  expected_set.insert(3ul);
  BOOST_REQUIRE( agent.set_num_workers_req() == expected_set ) ;
}
