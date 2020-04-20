#include <boost/test/unit_test.hpp>

#include <we/test/buffer_alignment/nets_using_buffers.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>
#include <drts/virtual_memory.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/virtual_memory_socket_name_for_localhost.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/program_options/validators/positive_integral.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#define START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET(NET)          \
  boost::program_options::options_description options_description;       \
  options_description.add (test::options::shared_directory());           \
  options_description.add (gspc::options::installation());               \
  options_description.add (gspc::options::drts());                       \
  options_description.add (gspc::options::scoped_rifd());                \
  options_description.add (gspc::options::virtual_memory());             \
                                                                         \
  boost::program_options::variables_map vm                               \
    ( test::parse_command_line                                           \
        ( boost::unit_test::framework::master_test_suite().argc          \
        , boost::unit_test::framework::master_test_suite().argv          \
        , options_description                                            \
        )                                                                \
    );                                                                   \
                                                                         \
  fhg::util::temporary_path const shared_directory                       \
    ( test::shared_directory (vm)                                        \
    / "buffer_alignment"                                                 \
    );                                                                   \
                                                                         \
  test::scoped_nodefile_from_environment const nodefile_from_environment \
    (shared_directory, vm);                                              \
                                                                         \
  fhg::util::temporary_path const _installation_dir                      \
    (shared_directory / boost::filesystem::unique_path());               \
  boost::filesystem::path const installation_dir (_installation_dir);    \
                                                                         \
  gspc::set_application_search_path (vm, installation_dir);              \
  test::set_virtual_memory_socket_name_for_localhost (vm);               \
                                                                         \
  vm.notify();                                                           \
                                                                         \
  gspc::installation const installation (vm);                            \
                                                                         \
  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}             \
                                 , gspc::rifd::hostnames {vm}            \
                                 , gspc::rifd::port {vm}                 \
                                 , installation                          \
                                 );                                      \
                                                                         \
  unsigned long local_memory_size (0);                                   \
                                                                         \
  fhg::util::temporary_path const _workflow_dir                          \
    (shared_directory / boost::filesystem::unique_path());               \
  boost::filesystem::path const workflow_dir (_workflow_dir);            \
                                                                         \
  boost::filesystem::ofstream                                            \
    (workflow_dir / (std::string (#NET) + ".xpnet"))                     \
    << we::test::buffer_alignment::NET (local_memory_size);              \
                                                                         \
  test::make_net_lib_install const make                                  \
    ( installation                                                       \
    , #NET                                                               \
    , workflow_dir                                                       \
    , installation_dir                                                   \
    );                                                                   \
                                                                         \
  gspc::scoped_runtime_system const drts                                 \
    ( vm                                                                 \
    , installation                                                       \
    , "worker:1," + std::to_string (local_memory_size)                   \
    , rifds.entry_points()                                               \
    )

BOOST_AUTO_TEST_CASE (arbitrary_buffer_sizes_and_alignments)
{
  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET
    (net_with_arbitrary_buffer_sizes_and_alignments);

  std::multimap<std::string, pnet::type::value::value_type> result;

  BOOST_REQUIRE_NO_THROW
    ( result = gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", we::type::literal::control()}})
    );

  decltype (result) const expected {{"done", we::type::literal::control()}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}

BOOST_AUTO_TEST_CASE (arbitrary_buffer_sizes_and_default_alignments)
{
  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET
    (net_with_arbitrary_buffer_sizes_and_default_alignments);

  std::multimap<std::string, pnet::type::value::value_type> result;

  BOOST_REQUIRE_NO_THROW
    ( result = gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", we::type::literal::control()}})
    );

  decltype (result) const expected {{"done", we::type::literal::control()}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}

BOOST_AUTO_TEST_CASE
  (arbitrary_buffer_sizes_and_alignments_insufficient_memory)
{
  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET
    (net_with_arbitrary_buffer_sizes_and_alignments_insufficient_memory);

  BOOST_REQUIRE_EXCEPTION
    ( gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", we::type::literal::control()}})
    , std::runtime_error
    , [](std::runtime_error const& exc)
      {
        return std::string (exc.what()).find
          ("Please take into account also the buffer alignments "
           "when allocating local shared memory!"
          ) != std::string::npos;
      }
    );
}
