#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

namespace
{
#define certificates_data                                                \
  boost::unit_test::data::make                                           \
    ( { gspc::certificates_t{}                                           \
      , gspc::certificates_t {GSPC_SSL_CERTIFICATES_FOR_TESTS}           \
      }                                                                  \
    )
}

BOOST_DATA_TEST_CASE
  (forbid_double_worker_instances, certificates_data, certificates)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::scoped_rifd());

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "forbid_double_worker_instances");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::scoped_rifds const rifds { gspc::rifd::strategy (vm)
                                 , gspc::rifd::hostnames (vm)
                                 , gspc::rifd::port (vm)
                                 , installation
                                 };

  gspc::scoped_runtime_system drts
    (vm, installation, "test_worker:1", rifds.entry_points(), certificates);

  std::unordered_map
    < gspc::rifd_entry_point
    , std::list<std::exception_ptr>
    , gspc::rifd_entry_point_hash
    > const errors (drts.add_worker (rifds.entry_points(), boost::none));

  BOOST_REQUIRE_EQUAL (rifds.hosts().size(), errors.size());

  for (auto const& exceptions : errors | boost::adaptors::map_values)
  {
    //! \todo do not collect the exceptions but make a longer list
    BOOST_REQUIRE (!exceptions.empty());
  }
}
