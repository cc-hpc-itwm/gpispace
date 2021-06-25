// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <fhg/util/boost/program_options/generic.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_file.hpp>

#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#define WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT_IMPL(workflow, topology) \
  namespace po = fhg::util::boost::program_options;                     \
                                                                        \
  po::option<po::nonempty_file> const jobserver_client                  \
    { "jobserver-client"                                                \
    , "the object that contains the jobserver client implementation"    \
    };                                                                  \
                                                                        \
  boost::program_options::options_description options_description;      \
                                                                        \
  options_description.add (test::options::source_directory());          \
  options_description.add (test::options::shared_directory());          \
  options_description.add (gspc::options::installation());              \
  options_description.add (gspc::options::drts());                      \
  options_description.add (gspc::options::scoped_rifd());               \
  options_description.add_options()                                     \
    ( jobserver_client.name().c_str()                                   \
    , boost::program_options::value<po::nonempty_file>()->required()    \
    , jobserver_client.description().c_str()                            \
    );                                                                  \
                                                                        \
  boost::program_options::variables_map vm                              \
    ( test::parse_command_line                                          \
      ( boost::unit_test::framework::master_test_suite().argc           \
      , boost::unit_test::framework::master_test_suite().argv           \
      , options_description                                             \
      )                                                                 \
    );                                                                  \
                                                                        \
  fhg::util::temporary_path const shared_directory                      \
    ( test::shared_directory (vm)                                       \
    / boost::unit_test::framework::current_test_case().full_name()      \
    );                                                                  \
                                                                        \
  test::scoped_nodefile_from_environment const                          \
    nodefile_from_environment (shared_directory, vm);                   \
                                                                        \
  fhg::util::temporary_path const _installation_dir                     \
    (shared_directory / boost::filesystem::unique_path());              \
  boost::filesystem::path const installation_dir (_installation_dir);   \
                                                                        \
  gspc::set_application_search_path (vm, installation_dir);             \
                                                                        \
  vm.notify();                                                          \
                                                                        \
  gspc::installation const installation (vm);                           \
                                                                        \
  test::make_net_lib_install const workflow                             \
    ( installation                                                      \
    , #workflow                                                         \
    , test::source_directory (vm) / "we" / "test" / "eureka" / "xpnet"  \
    , installation_dir                                                  \
    , test::option::options()                                           \
    . add<test::option::gen::include> (test::source_directory (vm))     \
    . add<test::option::gen::link> (jobserver_client.get_from (vm))     \
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
