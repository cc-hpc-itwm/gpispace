#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <util-generic/testing/random_integral.hpp>

namespace
{
  unsigned long bounded_random_positive_number (std::size_t bound)
  {
    return fhg::util::testing::random_integral_without_zero<unsigned long>() % bound;
  }

  we::type::activity_t activity (std::size_t const& size_buffer)
  {
    std::string const read_only_buffer
      (fhg::util::testing::random_string_without("\\\""));

    std::string const local
      ( ( boost::format
            ( "${range.buffer} := \"%1%\";"
              "${range.offset} := 0UL;"
              "${range.size} := %2%UL;"
              "stack_push (List(), ${range})"
            )
        % read_only_buffer
        % std::to_string (size_buffer)
        ).str()
      );

    std::string const global
      ( ( boost::format
            ( "${range.handle.name} := \"%1%\";"
              "${range.offset} := 0UL;"
              "${range.size} := %2%UL;"
              "stack_push (List(), ${range});"
            )
        % fhg::util::testing::random_string_without("\\\"")
        % std::to_string (size_buffer)
        ).str()
      );

    std::list<we::type::memory_transfer> memory_gets
      {{global, local, boost::none}};

    we::type::transition_t transition
      ( fhg::util::testing::random_string()
      , we::type::module_call_t ( fhg::util::testing::random_string()
                                , fhg::util::testing::random_string()
                                , {{read_only_buffer, {std::to_string (size_buffer) + "UL", true}}}
                                , std::move (memory_gets)
                                , std::list<we::type::memory_transfer>()
                                )
      , boost::none
      , we::type::property::type()
      , we::priority_type()
      );
    const std::string port_name (fhg::util::testing::random_string());
    transition.add_port ( we::type::port_t ( port_name
                                           , we::type::PORT_IN
                                           , std::string ("string")
                                           , we::type::property::type()
                                           )
                        );
    we::type::activity_t act (transition, boost::none);
    act.add_input ( transition.input_port_by_name (port_name)
                  , fhg::util::testing::random_string_without ("\\\"")
                  );
    return act;
  }
}

BOOST_FIXTURE_TEST_CASE
  ( attempting_to_transfer_const_data_into_a_shared_cache_with_smaller_size_fails
  , setup_logging
  )
{
  const boost::optional<intertwine::vmem::size_t>
    shared_cache_size (bounded_random_positive_number (200));

  utils::orchestrator const orchestrator (_logger);
  utils::agent const agent (orchestrator, shared_cache_size, _logger);
  utils::fake_drts_worker_directly_finishing_jobs const worker (agent);
  utils::client client (orchestrator);

  const std::size_t size_buffer
    (std::size_t (*shared_cache_size) + bounded_random_positive_number (100));

  auto const job (client.submit_job (activity (size_buffer)));

  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job, job_info), sdpa::status::FAILED);

  BOOST_REQUIRE_EQUAL
    ( job_info.error_message
    , agent.name()
    + ": Attempting to transfer read-only data from the global "
      "memory into a shared cache with insufficient size!"
    );
}

BOOST_FIXTURE_TEST_CASE
  ( attempting_to_transfer_const_data_into_a_non_existing_shared_cache_fails
  , setup_logging
  )
{
  const boost::optional<intertwine::vmem::size_t>
    shared_cache_size (bounded_random_positive_number (200));

  utils::orchestrator const orchestrator (_logger);
  utils::agent const agent (orchestrator, _logger);
  utils::fake_drts_worker_directly_finishing_jobs const worker (agent);
  utils::client client (orchestrator);

  const std::size_t size_buffer
    (bounded_random_positive_number (std::size_t (*shared_cache_size)));

  auto const job (client.submit_job (activity (size_buffer)));

  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job, job_info), sdpa::status::FAILED);

  BOOST_REQUIRE_EQUAL
    ( job_info.error_message
    , agent.name()
    + ": Attempting to transfer read-only data from the global "
      "memory into a non-existing shared cache!"
    );
}

BOOST_FIXTURE_TEST_CASE
  ( jobs_attempting_to_transfer_const_data_into_a_shared_cache_with_sufficient_size_are_sent_to_the_workers
  , setup_logging
  )
{
  const boost::optional<intertwine::vmem::size_t>
    shared_cache_size (bounded_random_positive_number (200));

  utils::orchestrator const orchestrator (_logger);
  utils::agent const agent (orchestrator, shared_cache_size, _logger);
  utils::fake_drts_worker_directly_finishing_jobs const worker (agent);
  utils::client client (orchestrator);

  const std::size_t size_buffer
    (bounded_random_positive_number (std::size_t (*shared_cache_size)));

  auto const job (client.submit_job (activity (size_buffer)));

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job), sdpa::status::FINISHED);
}
