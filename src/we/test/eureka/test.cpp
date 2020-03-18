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
#include <we/type/value/wrap.hpp>
#include <we/type/value/unwrap.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/program_options.hpp>

BOOST_AUTO_TEST_CASE (we_eureka_kill_all)
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
    (test::shared_directory (vm) / "we_eureka_kill_all");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "work:4", rifds.entry_points());

  test::make_net_lib_install const with_mod
    ( installation
    , "find_eureka_with_mod"
    , test::source_directory (vm)
    , installation_dir
    );
  test::make_net_lib_install const with_exp
    ( installation
    , "find_eureka_with_exp"
    , test::source_directory (vm)
    , installation_dir
    );

  for (auto workflow : {with_exp.pnet(), with_mod.pnet()})
  {
    BOOST_TEST_CONTEXT ("With eureka transition: " << workflow.filename())
    {
      for (unsigned long N (1UL); N <= 10UL; N+= 4)
      {
        BOOST_TEST_CONTEXT ("#workers: " << N)
        {
          std::multimap<std::string, pnet::type::value::value_type> tokens_on_port
            {{"eureka_gid", std::string ("find_small_value")}};

          for (unsigned long i (0); i < N; ++i)
          {
            tokens_on_port.emplace ("token", i);
          }

          auto const result
          ( gspc::client (drts).put_and_run
            (gspc::workflow (workflow), tokens_on_port)
          );
        }
      }
    }
  }
}
