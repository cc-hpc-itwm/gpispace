// Copyright (C) 2019-2024,2026 Fraunhofer ITWM
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
#include <gspc/testing/fmt_directory.hpp>

#include <gspc/util/read_lines.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/unique_path.hpp>

#include <gspc/testing/printer/we/type/value.hpp>

#include <boost/program_options.hpp>

#include <iterator>
#include <set>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (multiple_module_implementations)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::testing::options::fmt_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::scoped_rifd());

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  gspc::util::temporary_path const shared_directory
    ( gspc::testing::shared_directory (vm)
    / "multiple_module_implementations"
    );

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
    , "multiple_module_implementations"
    , gspc::testing::source_directory (vm)
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::include> (gspc::testing::source_directory (vm))
    . add<gspc::testing::option::gen::include>
        (gspc::testing::source_directory (vm).parent_path().parent_path().parent_path())
    . add<gspc::testing::option::gen::include>
        (gspc::testing::source_directory (vm).parent_path().parent_path().parent_path())
    . add<gspc::testing::option::gen::include> (std::filesystem::path {gspc::testing::fmt_directory (vm).string()} / "include")
    . add<gspc::testing::option::gen::cxx_flag> ("-DFMT_HEADER_ONLY")
    );

  std::vector<std::string> const hosts
    (gspc::util::read_lines (nodefile_from_environment.path()));

  BOOST_REQUIRE_GT (hosts.size(), 0);

  gspc::scoped_rifds const rifds
    ( gspc::rifd::strategy {vm}
    , gspc::rifd::hostnames {vm}
    , gspc::rifd::port {vm}
    , installation
    );

  std::set<std::string> const preferences {"A", "B", "C"};

  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , "A:20 B:20 C:20"
    , rifds.entry_points()
    );

  gspc::client client (drts);

  long const num_tasks (1000);
  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    (client.put_and_run
       ( gspc::workflow (make.pnet())
       , {{"num_tasks", num_tasks}}
       )
    );

  BOOST_REQUIRE_EQUAL (result.count ("done"), 1);

  auto const implementations (result.equal_range ("implementation"));
  BOOST_REQUIRE_EQUAL
    ( std::distance (implementations.first, implementations.second)
    , num_tasks
    );

  for (auto it (implementations.first); it != implementations.second; it++)
  {
    BOOST_REQUIRE (preferences.count (::boost::get<std::string>(it->second)));
  }
}

BOOST_AUTO_TEST_CASE (single_module_with_target)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::testing::options::fmt_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::scoped_rifd());

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  gspc::util::temporary_path const shared_directory
    ( gspc::testing::shared_directory (vm)
    / "multiple_module_implementations_single_module_with_target"
    );

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
    , "single_module_with_target"
    , gspc::testing::source_directory (vm)
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::include> (gspc::testing::source_directory (vm))
    . add<gspc::testing::option::gen::include>
        (gspc::testing::source_directory (vm).parent_path().parent_path().parent_path())
    . add<gspc::testing::option::gen::include> (std::filesystem::path {gspc::testing::fmt_directory (vm).string()} / "include")
    . add<gspc::testing::option::gen::cxx_flag> ("-DFMT_HEADER_ONLY")
    );

  std::vector<std::string> const hosts
    (gspc::util::read_lines (nodefile_from_environment.path()));

  BOOST_REQUIRE_GT (hosts.size(), 0);

  gspc::scoped_rifds const rifds
    ( gspc::rifd::strategy {vm}
    , gspc::rifd::hostnames {vm}
    , gspc::rifd::port {vm}
    , installation
    );

  std::set<std::string> const preferences {"A"};

  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , "A:20"
    , rifds.entry_points()
    );

  gspc::client client (drts);

  long const num_tasks (1000);
  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    (client.put_and_run
       ( gspc::workflow (make.pnet())
       , {{"num_tasks", num_tasks}}
       )
    );

  BOOST_REQUIRE_EQUAL (result.count ("done"), 1);

  auto const implementations (result.equal_range ("implementation"));
  BOOST_REQUIRE_EQUAL
    ( std::distance (implementations.first, implementations.second)
    , num_tasks
    );

  for (auto it (implementations.first); it != implementations.second; it++)
  {
    BOOST_REQUIRE (preferences.count (::boost::get<std::string>(it->second)));
  }
}
