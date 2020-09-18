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
#include <util-generic/testing/random.hpp>

#include <boost/program_options.hpp>
#include <algorithm>
#include <list>
#include <map>
#include <vector>
#include <string>

BOOST_AUTO_TEST_CASE (we_eureka_random_test)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
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
    (test::shared_directory (vm) / "we_eureka_random_test");

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
    (vm, "work:4", rifds.entry_points());

  test::make_net_lib_install const with_mod
    ( "find_eureka_with_mod"
    , test::source_directory (vm)
    , installation_dir
    );

  test::make_net_lib_install const with_exp
    ( "find_eureka_with_exp"
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
          auto const in_values
            (fhg::util::testing::unique_randoms<std::vector<long>> (N));
          auto const eureka
            (in_values.at (fhg::util::testing::random<unsigned>{}() % N));

          std::multimap<std::string, pnet::type::value::value_type>
            tokens_on_port
              { {"eureka_gid", std::string ("find_small_value")}
              , {"eureka_value", eureka}
              };

          for (auto const &i : in_values)
          {
            tokens_on_port.emplace ("token", i);
          }

          auto const result
          ( gspc::client (drts).put_and_run
            (gspc::workflow (workflow), tokens_on_port)
          );

          BOOST_REQUIRE (result.size() >= 1);
          BOOST_REQUIRE (result.count ("result") >= 1);

          int num_eurekas = 0;
          auto const responses = result.equal_range ("result");

          //!\note there may be one or more eureka responses for
          //!\note mod.xpnet; eureka value will be output by only one
          for (auto i = responses.first; i != responses.second; ++i)
          {
            auto const result_values
              ( boost::get<std::list<pnet::type::value::value_type>>
                (i->second)
              );

            if (result_values.size() == 1)
            {
              num_eurekas++;
              std::list<long> eureka_response
                (pnet::type::value::unwrap<long> (result_values));

              BOOST_CHECK_EQUAL ( eureka_response.front()
                                , eureka
                                );
            }
          }

          BOOST_CHECK_EQUAL (num_eurekas, 1);
        }
      }
    }
  }
}
