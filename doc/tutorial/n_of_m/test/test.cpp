// mirko.rahn@itwm.fraunhofer.de

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

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/program_options.hpp>

#include <map>

namespace
{
  void check (int num_worker)
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
      ( test::shared_directory (vm)
      / ("tutorial_n_of_m_" + std::to_string (num_worker))
      );

    test::scoped_nodefile_from_environment const nodefile_from_environment
      (shared_directory, vm);

    fhg::util::temporary_path const _installation_dir
      (shared_directory / boost::filesystem::unique_path());
    boost::filesystem::path const installation_dir (_installation_dir);

    gspc::set_application_search_path (vm, installation_dir);

    vm.notify();

    gspc::installation const installation (vm);

    test::make_net_lib_install const make
      ( installation
      , "n_of_m"
      , test::source_directory (vm)
      , installation_dir
      , test::option::options()
      . add<test::option::include> (test::source_directory (vm) / "..")
      );

    pnet::type::value::value_type config;
    pnet::type::value::poke ("description", config, std::string ("test"));

    gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                   , gspc::rifd::hostnames {vm}
                                   , gspc::rifd::port {vm}
                                   , installation
                                   );
    gspc::scoped_runtime_system const drts ( vm
                                           , installation
                                           , "work:" + std::to_string (num_worker)
                                           , rifds.entry_points()
                                           );

    std::multimap<std::string, pnet::type::value::value_type> const result
      ( gspc::client (drts).put_and_run
        ( gspc::workflow (make.pnet())
        , { {"n", 12L}
          , {"c", 4L}
          , {"config", config}
          }
        )
      );

    BOOST_REQUIRE_EQUAL (result.size(), 1);

    std::string const port_done ("done");

    BOOST_REQUIRE_EQUAL (result.count (port_done), 1);
    BOOST_CHECK_EQUAL
      ( result.find (port_done)->second
      , pnet::type::value::value_type (we::type::literal::control())
      );
  }
}

BOOST_AUTO_TEST_CASE (tutorial_n_of_m_seq)
{
  check (1);
}

BOOST_AUTO_TEST_CASE (tutorial_n_of_m_par)
{
  check (8);
}
