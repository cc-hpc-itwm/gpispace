#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>
#include <drts/virtual_memory.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>
#include <test/virtual_memory_socket_name_for_localhost.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>
#include <we/type/value/peek_or_die.hpp>

#include <util-generic/divru.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/random_integral.hpp>
#include <util-generic/testing/random_string.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <future>
#include <map>

BOOST_AUTO_TEST_CASE
  (attempting_to_transfer_const_data_in_a_shared_cache_with_smaller_size_fails)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (gspc::options::virtual_memory());

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "drts_write_only_shared_caches");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);
  test::set_virtual_memory_socket_name_for_localhost (vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( installation
    , "module_using_read_only_buffers"
    , test::source_directory (vm)
    , installation_dir
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  std::size_t const shared_cache_size
    ( fhg::util::testing::random_integral_without_zero<unsigned long>()
    % 100
    );

  gspc::scoped_runtime_system const drts
    (vm, installation, "worker:1,100", rifds.entry_points(), shared_cache_size);

  std::size_t const buffer_size
    ( fhg::util::divru
        ( shared_cache_size
        + fhg::util::testing::random_integral_without_zero<unsigned long>() % 100
        , rifds.hosts().size()
        )
    * rifds.hosts().size()
    );

  gspc::scoped_vmem_segment_and_allocation const allocation
    ( drts.alloc ( gspc::vmem::gaspi_segment_description()
                 , buffer_size
                 )
    );

  gspc::client client (drts);
  gspc::job_id_t const job_id
    ( client.submit ( gspc::workflow (make.pnet())
                    , { {"global_range", allocation.global_memory_range()}
                      , {"buffer_size", buffer_size}
                      }
                    )
    );

  BOOST_REQUIRE_EXCEPTION
    ( client.wait (job_id)
    , std::runtime_error
    , [&] (std::runtime_error const& exc)
      {
        return std::string (exc.what()).find
          ( "Attempting to transfer read-only data from the global "
            "memory into a shared cache with insufficient size!"
          ) != std::string::npos;
      }
    );
}

BOOST_AUTO_TEST_CASE
  (attempting_to_transfer_const_data_in_an_unspecified_shared_cache_fails)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (gspc::options::virtual_memory());

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "drts_write_only_shared_caches");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);
  test::set_virtual_memory_socket_name_for_localhost (vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( installation
    , "module_using_read_only_buffers"
    , test::source_directory (vm)
    , installation_dir
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system const drts
    (vm, installation, "worker:1,100", rifds.entry_points());

  std::size_t const buffer_size
    ( fhg::util::divru
        ( fhg::util::testing::random_integral_without_zero<unsigned long>()
        % 100
        , rifds.hosts().size()
        )
    * rifds.hosts().size()
    );

  gspc::scoped_vmem_segment_and_allocation const allocation
    ( drts.alloc ( gspc::vmem::gaspi_segment_description()
                 , buffer_size
                 )
    );

  gspc::client client (drts);
  gspc::job_id_t const job_id
    ( client.submit ( gspc::workflow (make.pnet())
                    , { {"global_range", allocation.global_memory_range()}
                      , {"buffer_size", buffer_size}
                      }
                    )
    );

  BOOST_REQUIRE_EXCEPTION
    ( client.wait (job_id)
    , std::runtime_error
    , [&] (std::runtime_error const& exc)
      {
        return std::string (exc.what()).find
          ( "Attempting to transfer read-only data from the global "
            "memory into a non-existing shared cache!"
          ) != std::string::npos;
      }
    );
}

BOOST_AUTO_TEST_CASE
  (attempting_to_transfer_const_data_in_a_shared_cache_with_enough_space_allocated_succeeds)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (gspc::options::virtual_memory());

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "drts_write_only_shared_caches");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);
  test::set_virtual_memory_socket_name_for_localhost (vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( installation
    , "module_using_read_only_buffers"
    , test::source_directory (vm)
    , installation_dir
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  std::size_t const shared_cache_size
    ( fhg::util::testing::random_integral_without_zero<unsigned long>()
    % 100
    );

  gspc::scoped_runtime_system const drts
    (vm, installation, "worker:1,100", rifds.entry_points(), shared_cache_size);

  std::size_t const buffer_size
    ( fhg::util::divru
        ( fhg::util::testing::random_integral_without_zero<unsigned long>()
        % shared_cache_size
        , rifds.hosts().size()
        )
    * rifds.hosts().size()
    );

  gspc::scoped_vmem_segment_and_allocation const allocation
    ( drts.alloc ( gspc::vmem::gaspi_segment_description()
                 , buffer_size
                 )
    );

  gspc::client client (drts);
  gspc::job_id_t const job_id
    ( client.submit ( gspc::workflow (make.pnet())
                    , { {"global_range", allocation.global_memory_range()}
                      , {"buffer_size", buffer_size}
                      }
                    )
    );

  std::multimap<std::string, pnet::type::value::value_type> const result
    (client.wait_and_extract (job_id));

   BOOST_REQUIRE_EQUAL (result.size(), 1);
   BOOST_REQUIRE_EQUAL (result.count ("result"), 1);
   BOOST_REQUIRE_EQUAL
     ( result.find ("result")->second
     , pnet::type::value::value_type (std::string ("executed"))
     );
}
