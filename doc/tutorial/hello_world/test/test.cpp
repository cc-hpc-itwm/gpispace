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
#include <we/type/value/poke.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

#include <map>

BOOST_AUTO_TEST_CASE (tutorial_hello_world)
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
    (test::shared_directory (vm) / "tutorial_hello_world");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  //! \todo allow more than one application search path, use a
  //! separate directory to build the modules...
  // fhg::util::temporary_path const _hello_world_module_dir
  //   (shared_directory / boost::filesystem::unique_path());
  //  boost::filesystem::path const sum_module_dir (_hello_world_module_dir);

  //! \todo ...instead of taking the installation directory
  boost::filesystem::path const sum_module_dir (_installation_dir);

  fhg::util::nest_exceptions<std::runtime_error>
    ([&sum_module_dir, &vm]()
     {
       std::ostringstream make_module;

       make_module
         << "make"
         << " BUILDDIR=" << sum_module_dir
         << " -C " << (test::source_directory (vm) / "src")
         ;

       fhg::util::system_with_blocked_SIGCHLD (make_module.str());
     }
    , "Could not 'make hello_world_module'"
    );

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( installation
    , "hello_many"
    , test::source_directory (vm)
    , installation_dir
    , test::option::options()
    . add<test::option::gen::link> (sum_module_dir / "hello2.o")
    . add<test::option::gen::link> (sum_module_dir / "hello_world.o")
    . add<test::option::gen::library_path> (sum_module_dir)
    . add<test::option::gen::include> (test::source_directory (vm) / "include")
    );

  pnet::type::value::value_type const control {we::type::literal::control()};

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "work:4", rifds.entry_points());

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"in", control}, {"in", control}})
    );

  decltype (result) const expected {{"out", control}, {"out", control}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
