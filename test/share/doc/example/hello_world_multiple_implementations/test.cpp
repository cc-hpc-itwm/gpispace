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

#include <gspc/we/type/value.hpp>
#include <gspc/testing/printer/we/type/value.hpp>
#include <gspc/we/type/value/poke.hpp>

#include <gspc/util/system_with_blocked_SIGCHLD.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#include <exception>
#include <map>
#include <stdexcept>

BOOST_AUTO_TEST_CASE (tutorial_hello_world)
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
    (gspc::testing::shared_directory (vm) / "tutorial_hello_world_multiple_implementations");

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  gspc::util::temporary_path const _installation_dir
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  auto const installation_dir {std::filesystem::path {_installation_dir}};

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  //! \todo allow more than one application search path, use a
  //! separate directory to build the modules...
  // gspc::util::temporary_path const _hello_world_module_dir
  //   (shared_directory / gspc::testing::unique_path());
  //  std::filesystem::path const sum_module_dir (_hello_world_module_dir);

  //! \todo ...instead of taking the installation directory
  auto const sum_module_dir {std::filesystem::path {_installation_dir}};

  try
  {
    std::ostringstream make_module;

    make_module
      << "make"
      << " BUILDDIR=" << sum_module_dir
      << " -C " << (gspc::testing::source_directory (vm) / "src")
      ;

    gspc::util::system_with_blocked_SIGCHLD (make_module.str());
  }
  catch (...)
  {
    std::throw_with_nested
      (std::runtime_error {"Could not 'make hello_world_module'"});
  }

  gspc::installation const installation (vm);

  gspc::testing::make_net_lib_install const make
    ( installation
    , "hello_world_multiple_implementations"
    , gspc::testing::source_directory (vm)
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::link> (sum_module_dir / "hello_world.o")
    . add<gspc::testing::option::gen::library_path> (sum_module_dir)
    . add<gspc::testing::option::gen::include> (gspc::testing::source_directory (vm) / "include")
    );

  gspc::pnet::type::value::value_type const control_v {gspc::we::type::literal::control()};

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "cpu:1 gpu:1", rifds.entry_points());

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"in", control_v}})
    );

  decltype (result) const expected {{"out", std::string ("gpu")}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
