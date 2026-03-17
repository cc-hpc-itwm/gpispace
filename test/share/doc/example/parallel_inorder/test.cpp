// Copyright (C) 2014-2015,2020-2024,2026 Fraunhofer ITWM
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
#include <gspc/we/type/value/poke.hpp>

#include <gspc/util/read_file.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#include <map>
#include <vector>

BOOST_AUTO_TEST_CASE (tutorial_parallel_inorder)
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
    (gspc::testing::shared_directory (vm) / "tutorial_parallel_inorder");

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

  gspc::testing::make_net_lib_install const make
    ( installation
    , "n_of_m"
    , gspc::testing::source_directory (vm)
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::include>
        (gspc::testing::source_directory (vm) / "..")
    );

  gspc::util::temporary_file const _output_file
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  std::filesystem::path const output_file (_output_file);

  gspc::pnet::type::value::value_type config;
  gspc::pnet::type::value::poke ("description", config, std::string ("test"));
  gspc::pnet::type::value::poke ("output_file", config, output_file.string());

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "work:5", rifds.entry_points());

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.pnet())
      , { {"n", 5L}
        , {"c", 5L}
        , {"config", config}
        }
      )
    );

  decltype (result) const expected {{"done", gspc::we::type::literal::control()}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);

  std::vector<char> const expected_file_content
    { 0, 0, 0
    , 1, 1, 1
    , 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
    , 3, 3, 3, 3, 3, 3
    , 4
    };

  std::string const file_content (gspc::util::read_file (output_file));

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    ( expected_file_content.begin(), expected_file_content.end()
    , file_content.begin(), file_content.end()
    );
}
