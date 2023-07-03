// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/source_directory.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <we/plugin/Base.hpp>

#include <util-generic/boost/program_options/generic.hpp>
#include <util-generic/boost/program_options/validators/existing_path.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <stdexcept>

BOOST_AUTO_TEST_CASE (exception_from_plugin_ctor_is_transported_to_users)
{
  namespace po = fhg::util::boost::program_options;

  po::option<po::existing_path> const option_plugin_path
    { "plugin-path"
    , "the path to the plugin with throwing ctor"
    };

  ::boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( option_plugin_path.name().c_str()
    , ::boost::program_options::value<po::existing_path>()->required()
    , option_plugin_path.description().c_str()
    );

  ::boost::program_options::variables_map vm
    ( test::parse_command_line
      ( ::boost::unit_test::framework::master_test_suite().argc
      , ::boost::unit_test::framework::master_test_suite().argv
      , options_description
      )
    );

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / ::boost::unit_test::framework::current_test_case().full_name()
    );

  test::scoped_nodefile_from_environment const
    nodefile_from_environment (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / ::boost::filesystem::unique_path());
  ::boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net const workflow
    ( installation
    , "create_plugin"
    , test::source_directory (vm)
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
  ::pnet::type::value::value_type const plugin_path_value
      (plugin_path.string());

  auto const job
    (client.submit (workflow.pnet(), {{"plugin_path", plugin_path_value}}));

  gspc::we::plugin::Context context;
  context.bind_ref ({"plugin_path"}, plugin_path_value);

  fhg::util::testing::require_exception
    ( [&]
      {
        client.wait (job);
      }
    , std::runtime_error
      (str ( ::boost::format ("Job %1%: failed: error-message :="
                           " workflow interpretation:"
                           " Plugins::create (%2%, %3%):"
                           " Exception in gspc_we_plugin_create: D::D()"
                           )
           % job
           % plugin_path
           % context
           )
      )
    );
}
