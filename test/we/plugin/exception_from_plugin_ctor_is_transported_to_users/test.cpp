// Copyright (C) 2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/we/plugin/Base.hpp>

#include <gspc/util/boost/program_options/generic.hpp>
#include <gspc/util/boost/program_options/validators/existing_path.hpp>

#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <gspc/we/expr/eval/context.formatter.hpp>
#include <filesystem>
#include <fmt/core.h>
#include <stdexcept>

BOOST_AUTO_TEST_CASE (exception_from_plugin_ctor_is_transported_to_users)
{
  namespace po = gspc::util::boost::program_options;

  po::option<po::existing_path> const option_plugin_path
    { "plugin-path"
    , "the path to the plugin with throwing ctor"
    };

  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( option_plugin_path.name().c_str()
    , ::boost::program_options::value<po::existing_path>()->required()
    , option_plugin_path.description().c_str()
    );

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
      ( ::boost::unit_test::framework::master_test_suite().argc
      , ::boost::unit_test::framework::master_test_suite().argv
      , options_description
      )
    );

  gspc::util::temporary_path const shared_directory
    ( gspc::testing::shared_directory (vm)
    / ::boost::unit_test::framework::current_test_case().full_name()
    );

  gspc::testing::scoped_nodefile_from_environment const
    nodefile_from_environment (shared_directory, vm);

  gspc::util::temporary_path const _installation_dir
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  auto const installation_dir {std::filesystem::path {_installation_dir}};

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net const workflow
    ( installation
    , "create_plugin"
    , gspc::testing::source_directory (vm)
      / "exception_from_plugin_ctor_is_transported_to_users"
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , "work:1"
    , rifds.entry_points()
    );

  gspc::client client {drts};

  auto const plugin_path (option_plugin_path.get_from (vm));
  gspc::pnet::type::value::value_type const plugin_path_value
      (static_cast<std::filesystem::path> (plugin_path).string());

  auto const job
    (client.submit (workflow.pnet(), {{"plugin_path", plugin_path_value}}));

  gspc::we::plugin::Context context;
  context.bind_ref ({"plugin_path"}, plugin_path_value);

  gspc::testing::require_exception
    ( [&]
      {
        client.wait (job);
      }
    , std::runtime_error
      { fmt::format ( "Job {}: failed: error-message :="
                      " workflow interpretation:"
                      " Plugins::create ({}, {}):"
                      " Exception in gspc_we_plugin_create: D::D()"
                    , job
                    , static_cast<std::filesystem::path> (plugin_path)
                    , context
                    )
      }
    );
}
