// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/source_directory.hpp>

#include <we/expr/eval/context.hpp>
#include <we/type/value.hpp>

#include <util-generic/read_lines.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/program_options.hpp>

#include <algorithm>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (plugin_create_and_call_after_in_one_transition)
{
  ::boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  options_description.add_options()
    ( "plugin-path"
    , ::boost::program_options::value<std::string>()->required()
    , "plugin path"
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
    / "plugin_create_and_call_after_in_one_transition"
    );

  auto const _log_path
    (static_cast<::boost::filesystem::path> (shared_directory) / "log");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net const make
    ( installation
    , "create_and_call_after_in_one_transition"
    , test::source_directory (vm)
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "work:1", rifds.entry_points());

  pnet::type::value::value_type const plugin_path
    (vm["plugin-path"].as<std::string>());
  pnet::type::value::value_type const log_path (_log_path.string());

  auto const result
    ( gspc::client (drts).put_and_run
        ( gspc::workflow (make.pnet())
        , { {"plugin_path", plugin_path}
          , {"log_path", log_path}
          }
        )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 1);
  BOOST_REQUIRE_EQUAL (result.count ("done"), 1);

  // \note context is unordered -> compare the sorted collections
  std::string const output
    ( [&]
      {
        std::ostringstream oss;
        expr::eval::context context;

        context.bind_ref ("plugin_path", plugin_path);
        context.bind_ref ("log_path", log_path);
        context.bind_and_discard_ref ({"plugin", "Log", "file"}, log_path);
        context.bind_and_discard_ref
          ({"plugin_id"}, result.find ("done")->second);

        oss << "after_eval\n" << context << '\n';

        return oss.str();
      }()
    );

  std::vector<std::string> expected;
  std::regex const sep ("\\n");
  std::copy ( std::sregex_token_iterator (output.begin(), output.end(), sep, -1)
            , std::sregex_token_iterator()
            , std::back_inserter (expected)
            );

  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION
    (expected, fhg::util::read_lines (_log_path));
}
