#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/program_options.hpp>

BOOST_AUTO_TEST_CASE (we_put_many_decomposes_result_lists)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "we_put_many_decomposes_result_lists");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 );
  gspc::scoped_runtime_system const drts
    (vm, "work:1", rifds.entry_points());

  test::make_net_lib_install const manual
    ( "manual"
    , test::source_directory (vm)
    , installation_dir
    );
  test::make_net_lib_install const automatic
    ( "automatic"
    , test::source_directory (vm)
    , installation_dir
    );

  for (auto workflow : {manual.pnet(), automatic.pnet()})
  {
    BOOST_TEST_CONTEXT ("With pnet type = " << workflow.filename())
    {
      for (unsigned long N (0UL); N < 10UL; ++N)
      {
        BOOST_TEST_CONTEXT ("With sum range = [0.." << N << ")")
        {
          std::multimap<std::string, pnet::type::value::value_type> const result
            ( gspc::client (drts).put_and_run
                (gspc::workflow (workflow), {{"N", N}})
            );

          decltype (result) const expected {{"S", (N * (N - 1UL)) / 2UL}};
          FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
        }
      }
    }
  }
}
