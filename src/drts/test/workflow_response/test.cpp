// bernd.loerwald@itwm.fraunhofer.de

#define BOOST_TEST_MODULE drts_workflow_response
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/wait_and_collect_exceptions.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <map>
#include <future>

#include <iostream>

BOOST_AUTO_TEST_CASE (workflow_response)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
      ( boost::unit_test::framework::master_test_suite().argc
      , boost::unit_test::framework::master_test_suite().argv
      )
    . options (options_description).run()
    , vm
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "workflow_response");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "workflow_response"
    , test::source_directory (vm)
    , {{"LIB_DESTDIR", installation_dir.string()}}
    , "net lib install"
    );

  gspc::scoped_rifds const rifds { gspc::rifd::strategy (vm)
                                 , gspc::rifd::hostnames (vm)
                                 , gspc::rifd::port (vm)
                                 , installation
                                 };

  gspc::scoped_runtime_system const drts
    (vm, installation, "worker:2", rifds.entry_points());

  gspc::client client (drts);

  gspc::workflow workflow (make.build_directory() / "workflow_response.pnet");

  workflow.set_wait_for_output();

  unsigned long const initial_state (0);

  gspc::job_id_t const job_id
    (client.submit (workflow, {{"state", initial_state}}));

  unsigned long value (initial_state);

  for (unsigned long i (0); i < 10; ++i)
  {
    BOOST_REQUIRE_EQUAL
      ( pnet::type::value::value_type (value)
      , client.synchronous_workflow_response (job_id, "get_and_update_state", i)
      );

    value += i;
  }

  std::future<pnet::type::value::value_type> asynchronous_response
    ( std::async
      ( std::launch::async
      , [&client, &job_id]()
        {
          return client.synchronous_workflow_response
            (job_id, "get_and_update_state", 0UL);
        }
      )
    );

  BOOST_REQUIRE_EQUAL ( pnet::type::value::value_type (value)
                      , asynchronous_response.get()
                      );

  //! \todo specific exception
  fhg::util::testing::require_exception<std::runtime_error>
    ([&client, &job_id]()
     {
       client.synchronous_workflow_response
         ("JOB-NOT-EXISTENT", "get_and_update_state", 12UL);
     }
    , "Error: reason := unable to put token: JOB-NOT-EXISTENT unknown or not running code := 3"
    );


  //! \note BUG: Hangs, GenericDaemon: unhandled error (3)
#if 0
  //! \todo specific exception
  fhg::util::testing::require_exception<std::runtime_error>
    ([&client, &job_id]()
     {
       client.synchronous_workflow_response
         (job_id, "get_and_update_state", std::string ("WRONG TYPE"));
     }
    , "Error: reason := unable to put token: type mismatch for field 'get_and_update_state': expected type 'unsigned long', value '\"WRONG TYPE\"' has type 'string'"
    );
#endif

  //! \note BUG: Hangs, GenericDaemon: unhandled error (3)
#if 0
  //! \todo specific exception
  fhg::util::testing::require_exception<std::runtime_error>
    ([&client, &job_id]()
     {
       client.synchronous_workflow_response
         (job_id, "PLACE-NOT-EXISTENT", 12UL);
     }
    , "Error: reason := unable to put token: unknown place PLACE-NOT-EXISTENT"
    );
#endif

  //! \note BUG: Hangs, GenericDaemon: unhandled error (3)
#if 0
  //! \todo specific exception
  fhg::util::testing::require_exception<std::runtime_error>
    ([&client, &job_id]()
     {
       client.synchronous_workflow_response
         (job_id, "state", 12UL);
     }
    , "Error: reason := unable to put token: place not marked with attribute put_token=\"true\""
    );
#endif

  client.put_token (job_id, "done", we::type::literal::control());
  client.wait (job_id);

  fhg::util::testing::require_exception<std::runtime_error>
    ([&client, &job_id]()
     {
       client.synchronous_workflow_response
         (job_id, "get_and_update_state", 0UL);
     }
    , "Error: reason := unable to put token: " + job_id + " unknown or not running code := 3"
    );

  std::multimap<std::string, pnet::type::value::value_type> const result
    (client.extract_result_and_forget_job (job_id));

  std::string const port_done ("done");
  std::string const port_state ("state");

  BOOST_REQUIRE_EQUAL (result.count (port_done), 1);
  BOOST_REQUIRE_EQUAL
    ( result.find (port_done)->second
    , pnet::type::value::value_type (we::type::literal::control())
    );
  BOOST_REQUIRE_EQUAL (result.count (port_state), 1);
  BOOST_REQUIRE_EQUAL ( result.find (port_state)->second
                      , pnet::type::value::value_type (value)
                      );
}

BOOST_AUTO_TEST_CASE (one_response_waits_while_others_are_made)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
      ( boost::unit_test::framework::master_test_suite().argc
      , boost::unit_test::framework::master_test_suite().argv
      )
    . options (options_description).run()
    , vm
    );

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / "workflow_response_one_response_waits_while_others_are_made"
    );

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "workflow_response_one_response_waits_while_others_are_made"
    , test::source_directory (vm)
    , {{"LIB_DESTDIR", installation_dir.string()}}
    , "net lib install"
    );

  gspc::scoped_rifds const rifds { gspc::rifd::strategy (vm)
                                 , gspc::rifd::hostnames (vm)
                                 , gspc::rifd::port (vm)
                                 , installation
                                 };

  gspc::scoped_runtime_system const drts
    (vm, installation, "work:2 management:1", rifds.entry_points());

  gspc::client client (drts);

  gspc::workflow workflow
    ( make.build_directory()
    / "workflow_response_one_response_waits_while_others_are_made.pnet"
    );

  workflow.set_wait_for_output();

  unsigned long const initial_state (0);
  std::atomic<unsigned long> status_updates (0);
  unsigned long const threads (2);

  std::mt19937_64 eng (std::random_device{}());
  std::uniform_int_distribution<unsigned long> dist (20, 50);

  gspc::job_id_t const job_id
    ( client.submit ( workflow
                    , { {"state", initial_state}
                      , {"random_module_calls", dist (eng)}
                      }
                    )
    );

  std::mutex no_longer_do_status_update_guard;
  bool no_longer_do_status_update (false);

  std::future<void> done_check
    ( std::async ( std::launch::async
                 , [ &client, &job_id, &no_longer_do_status_update
                   , &no_longer_do_status_update_guard
                   ]
                   {
                     client.synchronous_workflow_response
                       (job_id, "check_done_trigger", 0UL);

                     no_longer_do_status_update = true;
                     {
                       std::unique_lock<std::mutex> const _
                         (no_longer_do_status_update_guard);

                       client.put_token
                         (job_id, "done_done", we::type::literal::control());
                     }

                     client.wait (job_id);
                   }
                 )
    );

  auto&& thread_function
    ( [ &status_updates, &job_id, &drts, &no_longer_do_status_update
      , &no_longer_do_status_update_guard, &eng
      ]
      {
        unsigned long updates (0);
        while (true)
        {
          std::unique_lock<std::mutex> const _ (no_longer_do_status_update_guard);
          if (no_longer_do_status_update)
          {
            break;
          }

          gspc::client (drts).synchronous_workflow_response
            (job_id, "get_and_update_state_trigger", 1UL);
          ++updates;
        }
        status_updates += updates;
      }
    );

  {
    std::vector<std::future<void>> results;
    for (unsigned long i (0); i < threads; ++i)
    {
      results.emplace_back (std::async (std::launch::async, thread_function));
    }

    fhg::util::wait_and_collect_exceptions (results);
  }

  done_check.get();

  std::multimap<std::string, pnet::type::value::value_type> const result
    (client.extract_result_and_forget_job (job_id));

  std::string const port_done ("done");
  std::string const port_state ("state");

  BOOST_REQUIRE_EQUAL (result.count (port_done), 1);
  BOOST_REQUIRE_EQUAL
    ( result.find (port_done)->second
    , pnet::type::value::value_type (we::type::literal::control())
    );
  BOOST_REQUIRE_EQUAL (result.count (port_state), 1);
  BOOST_REQUIRE_EQUAL ( result.find (port_state)->second
                      , pnet::type::value::value_type
                          (initial_state + status_updates)
                      );
}
