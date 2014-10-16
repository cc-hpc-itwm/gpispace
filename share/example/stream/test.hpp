// mirko.rahn@itwm.fraunhofer.de

#ifndef SHARE_EXAMPLE_STREAM_TEST_HPP
#define SHARE_EXAMPLE_STREAM_TEST_HPP

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/stream.hpp>
#include <drts/virtual_memory.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_with_localhost.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>
#include <test/virtual_memory_socket_name_for_localhost.hpp>

#include <we/type/value.hpp>

#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/temporary_file.hpp>
#include <fhg/util/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/test/unit_test.hpp>

#include <chrono>
#include <functional>
#include <string>
#include <thread>

namespace share_example_stream_test
{
  template<typename R, typename P>
  void run
    ( std::string const& workflow_name
    , std::function<std::string (unsigned long size_slot)> const& topology
    , std::chrono::duration<R, P> const& sleep_after_produce
    )
  {
    namespace validators = fhg::util::boost::program_options;

    boost::program_options::options_description options_description;

    constexpr char const* const option_num_slots ("num-slots");
    constexpr char const* const option_size_slot ("size-slot");

    options_description.add_options()
      ( option_num_slots
      , boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
      , "number of slots in virtual memory"
      )
      ( option_size_slot
      , boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
      , "size of slots in virtual memory"
      )
      ;

    options_description.add (test::options::shared_directory());
    options_description.add (test::options::source_directory());
    options_description.add (gspc::options::installation());
    options_description.add (gspc::options::drts());
    options_description.add (gspc::options::virtual_memory());

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
      (test::shared_directory (vm) / ("share_example_stream" + workflow_name));

    test::scoped_state_directory const state_directory (shared_directory, vm);
    test::scoped_nodefile_with_localhost const nodefile_with_localhost
      (shared_directory, vm);

    fhg::util::temporary_path const _installation_dir
      (shared_directory / boost::filesystem::unique_path());
    boost::filesystem::path const installation_dir (_installation_dir);

    gspc::set_application_search_path (vm, installation_dir);
    test::set_virtual_memory_socket_name_for_localhost (vm);

    vm.notify();

    fhg::util::temporary_file const temporary_file
      (shared_directory / boost::filesystem::unique_path());

    boost::filesystem::path const log_file (temporary_file);

    gspc::installation const installation (vm);

    test::make const make
      ( installation
      , workflow_name
      , test::source_directory (vm)
      , { {"LIB_DESTDIR", installation_dir.string()}
        , {"CXXINCLUDEPATHS", test::source_directory (vm).string()}
        }
      , "net lib install"
      );

    gspc::stream::number_of_slots const num_slots
      (vm[option_num_slots].as<validators::positive_integral<unsigned long>>());
    gspc::stream::size_of_slot const size_slot
      (vm[option_size_slot].as<validators::positive_integral<unsigned long>>());

    unsigned long const size (num_slots * size_slot);

    gspc::scoped_runtime_system const drts
      (vm, installation, topology (size_slot));

    gspc::vmem_allocation const allocation_buffer
      (drts.alloc (size, workflow_name + "_buffer"));
    gspc::vmem_allocation const allocation_meta
      (drts.alloc ( num_slots * gspc::stream::size_of_meta_data_slot()
                  , workflow_name + "_meta"
                  )
      );
    gspc::client client (drts);

    gspc::workflow workflow
      (make.build_directory() / (workflow_name + ".pnet"));

    workflow.set_wait_for_output();

    gspc::job_id_t const job_id
      (client.submit (workflow, {{"log_file", log_file.string()}}));

    gspc::stream stream
      (drts.create_stream ( "stream_test"
                          , allocation_buffer
                          , allocation_meta
                          , size_slot
                          , num_slots
                          , [&client, &job_id] (pnet::type::value::value_type const& value) -> void
                            {
                              client.put_token (job_id, "work_package", value);
                            }
                          )
      );

    std::chrono::high_resolution_clock clock;

    std::ostringstream expected_output;

    for (unsigned long id (0); id < 20 * num_slots; ++id)
    {
      std::chrono::high_resolution_clock::rep const now
        ( std::chrono::duration_cast<std::chrono::microseconds>
          (clock.now().time_since_epoch()).count()
        );

      std::string const data ((boost::format ("%1% %2%") % id % now).str());

      expected_output << data << std::endl;

      stream.write (data);

      std::this_thread::sleep_for (sleep_after_produce);
    }

    client.put_token (job_id, "stop", we::type::literal::control());

    std::multimap<std::string, pnet::type::value::value_type> const result
      (client.wait_and_extract (job_id));

    BOOST_REQUIRE_EQUAL (result.count ("done"), 1);

    BOOST_REQUIRE_EQUAL
      (expected_output.str(), fhg::util::read_file (log_file));
  }
}

#endif
