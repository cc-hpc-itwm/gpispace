// Copyright (C) 2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/util/boost/program_options/generic.hpp>
#include <gspc/util/boost/program_options/validators/nonempty_file.hpp>

#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#define WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT_IMPL(workflow, topology) \
  namespace po = gspc::util::boost::program_options;                     \
                                                                        \
  po::option<po::nonempty_file> const jobserver_client                  \
    { "jobserver-client"                                                \
    , "the object that contains the jobserver client implementation"    \
    };                                                                  \
                                                                        \
  ::boost::program_options::options_description options_description;      \
                                                                        \
  options_description.add (gspc::testing::options::source_directory());          \
  options_description.add (gspc::testing::options::shared_directory());          \
  options_description.add (gspc::options::installation());              \
  options_description.add (gspc::options::drts());                      \
  options_description.add (gspc::options::scoped_rifd());               \
  options_description.add_options()                                     \
    ( jobserver_client.name().c_str()                                   \
    , ::boost::program_options::value<po::nonempty_file>()->required()    \
    , jobserver_client.description().c_str()                            \
    );                                                                  \
                                                                        \
  ::boost::program_options::variables_map vm                              \
    ( gspc::testing::parse_command_line                                          \
      ( ::boost::unit_test::framework::master_test_suite().argc           \
      , ::boost::unit_test::framework::master_test_suite().argv           \
      , options_description                                             \
      )                                                                 \
    );                                                                  \
                                                                        \
  gspc::util::temporary_path const shared_directory                      \
    ( gspc::testing::shared_directory (vm)                                       \
    / ::boost::unit_test::framework::current_test_case().full_name()      \
    );                                                                  \
                                                                        \
  gspc::testing::scoped_nodefile_from_environment const                          \
    nodefile_from_environment (shared_directory, vm);                   \
                                                                        \
  gspc::util::temporary_path const _installation_dir                     \
    ( std::filesystem::path {shared_directory}                          \
    / gspc::testing::unique_path()                                          \
    );                                                                  \
  auto const installation_dir {std::filesystem::path {_installation_dir}};   \
                                                                        \
  gspc::set_application_search_path (vm, installation_dir);             \
                                                                        \
  vm.notify();                                                          \
                                                                        \
  gspc::installation const installation (vm);                           \
                                                                        \
  gspc::testing::make_net_lib_install const workflow                             \
    ( installation                                                      \
    , #workflow                                                         \
    , gspc::testing::source_directory (vm) / "test" / "we" / "eureka" / "xpnet"  \
    , installation_dir                                                  \
    , gspc::testing::option::options()                                           \
    . add<gspc::testing::option::gen::include> (gspc::testing::source_directory (vm))     \
    . add<gspc::testing::option::gen::link>                                      \
       ( static_cast<std::filesystem::path>                             \
          (jobserver_client.get_from (vm))                              \
       )                                                                \
    );                                                                  \
                                                                        \
  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}            \
                                 , gspc::rifd::hostnames {vm}           \
                                 , gspc::rifd::port {vm}                \
                                 , installation                         \
                                 );                                     \
                                                                        \
  gspc::scoped_runtime_system const drts                                \
    ( vm                                                                \
    , installation                                                      \
    , topology                                                          \
    , rifds.entry_points()                                              \
    );                                                                  \
                                                                        \
  gspc::client client (drts);                                           \
                                                                        \
  auto const job                                                        \
    ( client.submit ( workflow.pnet()                                   \
                    , { {"register_host", jobserver.host()}             \
                      , {"register_port", jobserver.port()}             \
                      }                                                 \
                    )                                                   \
    )

#define WE_TEST_EUREKA_PUT_TOKEN_IMPL(place, value)                     \
  client.put_token (job, place, value)

#define WE_TEST_EUREKA_GET_RESULT_IMPL()                                \
  client.extract_result_and_forget_job (job)

#define NUMBER_OF_NODES_IMPL()                                          \
  rifds.hosts().size()
