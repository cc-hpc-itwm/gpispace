// Copyright (C) 2019-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>

#include <gspc/we/type/value.hpp>
#include <gspc/testing/printer/we/type/value.hpp>

#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

BOOST_AUTO_TEST_CASE (we_put_many_decomposes_result_lists)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  gspc::util::temporary_path const shared_directory
    (gspc::testing::shared_directory (vm) / "we_put_many_decomposes_result_lists");

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  gspc::util::temporary_path const _installation_dir
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  auto const installation_dir {std::filesystem::path {_installation_dir}};

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "work:1", rifds.entry_points());

  gspc::testing::make_net_lib_install const manual
    ( installation
    , "manual"
    , gspc::testing::source_directory (vm)
    , installation_dir
    );
  gspc::testing::make_net_lib_install const automatic
    ( installation
    , "automatic"
    , gspc::testing::source_directory (vm)
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
          std::multimap<std::string, gspc::pnet::type::value::value_type> const result
            ( gspc::client (drts).put_and_run
                (gspc::workflow (workflow), {{"N", N}})
            );

          decltype (result) const expected {{"S", (N * (N - 1UL)) / 2UL}};
          GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
        }
      }
    }
  }
}
