#define BOOST_TEST_MODULE restart_drts_worker_while_jobs_are_running

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/test/unit_test.hpp>

#include <fhg/util/random_string.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

namespace
{
  bool has_state_pending (sdpa::discovery_info_t const& disc_res)
  {
   return disc_res.state() && disc_res.state().get() == sdpa::status::PENDING;
  }

  bool all_childs_are_pending (sdpa::discovery_info_t const& disc_res)
  {
   BOOST_FOREACH
     (const sdpa::discovery_info_t& child_info, disc_res.children())
   {
     if (!has_state_pending (child_info))
     {
       return false;
     }
   }

   return true;
  }

  bool has_children_and_all_children_are_pending
    (sdpa::discovery_info_t const& disc_res)
  {
    std::cout<<disc_res<<std::endl;
    return !disc_res.children().empty() && all_childs_are_pending (disc_res);
  }

  sdpa::events::ErrorEvent::Ptr create_disconnect_event (const sdpa::worker_id_t& worker_id,
                                                         const sdpa::worker_id_t& agent_id)
  {
    sdpa::events::ErrorEvent::Ptr pErrEvt(
          new  sdpa::events::ErrorEvent( worker_id
                                        , agent_id
                                        , sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
                                        , std::string("worker ")+worker_id+" went down!" ));
    return pErrEvt;
  }
}

BOOST_AUTO_TEST_CASE (restart_drts_worker_while_job_is_running_with_polling_client)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  sdpa::daemon::Agent agent ( "agent_0"
                              , "127.0.0.1",  kvs_host(), kvs_port(),
                              , sdpa::master_info_list_t (1, sdpa::MasterInfo ("orchestrator_0"))
                              , boost::none);

  const utils::drts_worker worker_0( "drts_0", "agent_0"
                             , ""
                             , TESTS_TRANSFORM_FILE_MODULES_PATH
                             , kvs_host(), kvs_port()
                             );

  while(!agent.hasWorker("drts_0"));

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t const job_id (client.submitJob (workflow));

  agent.handleErrorEvent(create_disconnect_event("drts_0", "agent_0").get());

  // wait until all remaining jobs are discovered pending
  while (!has_children_and_all_children_are_pending
        (client.discoverJobStates (fhg::util::random_string(), job_id))
          )
  {}

  const utils::drts_worker worker_new
    ( "drts_new", "agent_0"
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  sdpa::client::job_info_t job_info;

  BOOST_REQUIRE_EQUAL
     ( utils::client::wait_for_job_termination(client, job_id)
     , sdpa::status::FINISHED );
}

BOOST_AUTO_TEST_CASE (restart_drts_worker_while_job_is_running_with_subscribing_client)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  sdpa::daemon::Agent agent ( "agent_0"
                               , "127.0.0.1", kvs_host(), kvs_port()
                               , sdpa::master_info_list_t (1, sdpa::MasterInfo ("orchestrator_0"))
                               , boost::none);

  boost::scoped_ptr<utils::drts_worker> worker_0
    ( new utils::drts_worker ( "drts_0", "agent_0"
                             , ""
                             , TESTS_TRANSFORM_FILE_MODULES_PATH
                             , kvs_host(), kvs_port()
                             )
    );

  while(!agent.hasWorker("drts_0"));

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t const job_id (client.submitJob (workflow));

  agent.handleErrorEvent(create_disconnect_event("drts_0", "agent_0").get());

  // wait until all remaining jobs are discovered pending
  while (!has_children_and_all_children_are_pending
         (client.discoverJobStates (fhg::util::random_string(), job_id))
           )
   {}

  const utils::drts_worker worker_new
    ( "drts_new", "agent_0"
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
      ( client.wait_for_terminal_state (job_id, job_info)
      , sdpa::status::FINISHED );
}

BOOST_AUTO_TEST_CASE (restart_drts_worker_while_coallocated_job_is_running_with_subscribing_client)
{
  const std::string workflow
    (utils::require_and_read_file ("coallocation_test.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  sdpa::daemon::Agent agent ( "agent_0"
                               , "127.0.0.1", kvs_host(), kvs_port()
                               , sdpa::master_info_list_t (1, sdpa::MasterInfo ("orchestrator_0"))
                               , boost::none);

  const utils::drts_worker worker_one_cap_0
    ( "drts_one_cap_0", "agent_0"
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_one_cap_1
    ( "drts_one_cap_1", "agent_0"
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  const utils::drts_worker worker_both_caps_0
    ( "drts_both_caps_0", "agent_0"
    , "A,B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_both_caps_1
    ( "drts_both_caps_1", "agent_0"
    , "A,B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  const utils::drts_worker worker_both_caps_to_be_restarted
    ( "drts_both_caps_to_be_restarted", "agent_0"
    , "A,B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  while(!agent.hasWorker("drts_both_caps_to_be_restarted"));

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t const job_id (client.submitJob (workflow));

  agent.handleErrorEvent(create_disconnect_event("drts_both_caps_to_be_restarted", "agent_0").get());

  // wait until all remaining jobs are discovered pending
  while (!has_children_and_all_children_are_pending
         (client.discoverJobStates (fhg::util::random_string(), job_id))
           )
   {}

   utils::drts_worker worker_both_caps_restarted
    ( "drts_both_caps_restarted", "agent_0"
    , "A,B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
        ( client.wait_for_terminal_state (job_id, job_info)
        , sdpa::status::FINISHED );
}
