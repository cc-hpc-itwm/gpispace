// Copyright (C) 2014-2016,2020-2024,2026 Fraunhofer ITWM
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

#include <algorithm>
#include <exception>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <vector>

namespace
{
  void run_and_check
    ( ::boost::program_options::variables_map const& vm
    , gspc::installation const& installation
    , std::filesystem::path const& pnet
    )
  {
    gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                   , gspc::rifd::hostnames {vm}
                                   , gspc::rifd::port {vm}
                                   , installation
                                   );
    gspc::scoped_runtime_system const drts
      (vm, installation, "work:4", rifds.entry_points());

    auto pair
      ( [] (long x, long y) -> gspc::pnet::type::value::value_type
      {
        gspc::pnet::type::value::value_type v;
        gspc::pnet::type::value::poke ("x", v, x);
        gspc::pnet::type::value::poke ("y", v, y);
        return v;
      }
      );

    std::multimap<std::string, gspc::pnet::type::value::value_type> const result
      ( gspc::client (drts)
      . put_and_run ( gspc::workflow (pnet)
                    , {{"p", pair (3, 4)}, {"p", pair (-2, 3)}}
                    )
      );

    decltype (result) const expected {{"s", 1L}, {"s", 7L}};
    GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
  }
}

BOOST_AUTO_TEST_CASE (tutorial_sum_expr)
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
    (gspc::testing::shared_directory (vm) / "tutorial_sum_expr");

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net const make
    ( installation
    , "sum_expr_many"
    , gspc::testing::source_directory (vm)
    );

  run_and_check (vm, installation, make.pnet());
}

BOOST_AUTO_TEST_CASE (tutorial_sum_mod)
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
    (gspc::testing::shared_directory (vm) / "tutorial_sum_mod");

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  gspc::util::temporary_path const _installation_dir
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  auto const installation_dir {std::filesystem::path {_installation_dir}};

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::util::temporary_path const _sum_module_dir
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  auto const sum_module_dir {std::filesystem::path {_sum_module_dir}};

  try
  {
    std::ostringstream make_module;

    make_module
      << "make"
      << " DIR_BUILD=" << sum_module_dir
      << " -C " << (gspc::testing::source_directory (vm) / "src")
      ;

    gspc::util::system_with_blocked_SIGCHLD (make_module.str());
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error {"Could not 'make sum_module'"}
      );
  }

  gspc::installation const installation (vm);

  gspc::testing::make_net_lib_install const make
    ( installation
    , "sum_many"
    , gspc::testing::source_directory (vm)
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::link> (sum_module_dir / "sum.o")
    . add<gspc::testing::option::gen::include> (gspc::testing::source_directory (vm) / "include")
    );

  run_and_check (vm, installation, make.pnet());
}
