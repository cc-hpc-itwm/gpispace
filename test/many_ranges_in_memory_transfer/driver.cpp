#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>
#include <drts/virtual_memory.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>
#include <test/virtual_memory_socket_name_for_localhost.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <list>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (many_ranges_in_memory_transfer)
{
  boost::program_options::options_description options_description;

  constexpr char const* const S ("size-of-range");
  constexpr char const* const D ("distance-between-ranges");
  constexpr char const* const N ("number-of-ranges");

  using PositiveUL =
    fhg::util::boost::program_options::positive_integral<unsigned long>;

  options_description.add_options()
    (S, boost::program_options::value<PositiveUL>()->required())
    (D, boost::program_options::value<unsigned long>()->required())
    (N, boost::program_options::value<PositiveUL>()->required())
    ;
  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
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
    (test::shared_directory (vm) / "many_ranges_in_memory_transfer");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);
  test::set_virtual_memory_socket_name_for_localhost (vm);

  vm.notify();

  test::make_net_lib_install const make
    ( "many_ranges_in_memory_transfer"
    , test::source_directory (vm)
    , installation_dir
    );

  unsigned long const size_of_range (vm.at (S).as<PositiveUL>());
  unsigned long const distance_between_ranges (vm.at (D).as<unsigned long>());
  unsigned long const number_of_ranges (vm.at (N).as<PositiveUL>());

  auto const data
    (fhg::util::testing::randoms<std::vector<char>>
      (number_of_ranges * (size_of_range + distance_between_ranges))
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 );

  gspc::scoped_runtime_system const drts
    ( vm
    , "worker:1," + std::to_string (number_of_ranges * size_of_range)
    , rifds.entry_points()
    );

  gspc::vmem_allocation const allocation
    ( drts.alloc_and_fill ( gspc::vmem::gaspi_segment_description()
                          , data.size()
                          , "data"
                          , data.data()
                          )
    );

  std::list<pnet::type::value::value_type> global;

  for (unsigned long range (0); range < number_of_ranges; ++range)
  {
    global.emplace_back
      ( allocation.global_memory_range
          ( range * (size_of_range + distance_between_ranges)
          , size_of_range
          )
      );
  }

  auto const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.pnet())
      , { {"data", we::type::bytearray (data)}
        , {"global", global}
        , {"size_of_range", size_of_range}
        , {"distance_between_ranges", distance_between_ranges}
        , {"number_of_ranges", number_of_ranges}
        }
      )
    );

  decltype (result) const expected {{"out", we::type::literal::control()}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
