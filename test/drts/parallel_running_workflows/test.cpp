// Copyright (C) 2014-2016,2018-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/testing/certificates_data.hpp>
#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>

#include <gspc/we/type/value.hpp>

#include <gspc/util/temporary_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/optional.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>

#include <future>
#include <map>

BOOST_AUTO_TEST_CASE (drts_parallel_running_workflows)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( "ssl-cert"
    , ::boost::program_options::value<std::string>()->required()
    , "enable or disable SSL certificate"
    );

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  std::string const ssl_cert (vm.at ("ssl-cert").as<std::string>());


  gspc::util::temporary_path const shared_directory
    ( gspc::testing::shared_directory (vm)
    / ( "drts_parallel_running_workflows"
      + ssl_cert
      + "_cert"
      )
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

  gspc::util::temporary_file const temporary_file_a
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  gspc::util::temporary_file const temporary_file_b
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );

  std::filesystem::path const filename_a (temporary_file_a);
  std::filesystem::path const filename_b (temporary_file_b);

  gspc::testing::make_net_lib_install const make_wait_then_touch
    ( installation
    , "wait_then_touch"
    , gspc::testing::source_directory (vm)
    , installation_dir
    );
  gspc::testing::make_net_lib_install const make_touch_then_wait
    ( installation
    , "touch_then_wait"
    , gspc::testing::source_directory (vm)
    , installation_dir
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  auto const certificates ( ssl_cert  == "yes" ? gspc::testing::yes_certs()
                                               : gspc::testing::no_certs()
                          );

  gspc::scoped_runtime_system const drts
    (vm, installation, "worker:2", rifds.entry_points(), std::cerr, certificates);

  auto submit_fun
    ( [&filename_a, &filename_b, &drts, &certificates]
      (std::string port, gspc::testing::make_net_lib_install const& make)
    {
      std::multimap<std::string, gspc::pnet::type::value::value_type> const result
        ( gspc::client (drts, certificates).put_and_run
          ( gspc::workflow (make.pnet())
          , { {"filename_a", filename_a.string()}
            , {"filename_b", filename_b.string()}
            , {"timeout_in_seconds", 30U}
            }
          )
        );

      return result.count (port) == 1
        && result.find (port)->second == gspc::pnet::type::value::value_type (true);
    }
    );

  std::future<bool> wait_then_touch
    ( std::async ( std::launch::async
                 , submit_fun
                 , "a_existed"
                 , std::cref (make_wait_then_touch)
                 )
    );
  std::future<bool> touch_then_wait
    ( std::async ( std::launch::async
                 , submit_fun
                 , "b_existed"
                 , std::cref (make_touch_then_wait)
                 )
    );

  BOOST_CHECK (wait_then_touch.get());
  BOOST_CHECK (touch_then_wait.get());
}
