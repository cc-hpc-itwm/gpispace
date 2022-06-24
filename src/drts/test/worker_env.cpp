// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <we/type/value/boost/test/printer.hpp>
#include <we/type/value/wrap.hpp>

#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/source_directory.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/framework.hpp>
#include <boost/test/unit_test_suite.hpp>

#include <array>
#include <fstream>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace
{
  std::vector<std::string> merge_with_cmdline_args
    (std::vector<std::string> extra_args)
  {
    std::vector<std::string> args
      ( ::boost::unit_test::framework::master_test_suite().argv
      , ::boost::unit_test::framework::master_test_suite().argv
      + ::boost::unit_test::framework::master_test_suite().argc
      );
    for (auto&& arg : std::move (extra_args))
    {
      args.emplace_back (std::move (arg));
    }
    return args;
  }

  //! - build `report_environment` and provide as `pnet = make.pnet()`
  struct compile_pnet
  {
    compile_pnet()
    {
      ::boost::program_options::variables_map vm;
      ::boost::program_options::store
        ( ::boost::program_options::command_line_parser
            ( ::boost::unit_test::framework::master_test_suite().argc
            , ::boost::unit_test::framework::master_test_suite().argv
            )
        . options ( ::boost::program_options::options_description()
                  . add (test::options::source_directory())
                  . add (test::options::shared_directory())
                  . add (gspc::options::installation())
                  )
        . allow_unregistered()
        . run()
        , vm
        );
      vm.notify();

      app_search_path
        = test::shared_directory (vm) / ::boost::filesystem::unique_path();

      _make.emplace ( vm
                    , "report_environment"
                    , test::source_directory (vm) / "worker_env"
                    , app_search_path
                    );
      pnet = _make->pnet();
    }

    static ::boost::filesystem::path app_search_path;
    static ::boost::filesystem::path pnet;

    ::boost::optional<test::make_net_lib_install> _make;
  };

  ::boost::filesystem::path compile_pnet::app_search_path = {};
  ::boost::filesystem::path compile_pnet::pnet = {};

  using random_string = fhg::util::testing::random<std::string>;

  struct random_env_kvpair
  {
    static std::string random_key_that_does_not_interfere()
    {
      // List assembled via `git grep 'getenv' | grep '"'`.
      std::string key;
      do
      {
        key = random_string{} (random_string::identifier{});
      }
      while ( // Used by rif strategy ssh
              key == "USER"
              // Used by rif strategy ssh
           || key == "HOME"
              // Used by test::scoped_nodefile_from_environment
           || key == "GSPC_NODEFILE_FOR_TESTS"
            );
      return key;
    }

    random_env_kvpair()
      : key (random_key_that_does_not_interfere())
      , value ( random_string{}
                  (random_string::except (std::string (1, '\0') + "\n"))
              )
      , definition (key + "=" + value)
    {}

    std::string const key;
    std::string const value;
    std::string const definition;
  };
}

BOOST_GLOBAL_FIXTURE (compile_pnet);

//! - command line parsing `vm` with arguments from the actual command
//!   line, merged with the vector<string> \a extra_args_
//! - `shared_directory` + `lib_install_directory` + nodefile
//! - rifds + 1 worker "worker" per node
//! - `client`
//! - use the global pnet compilation to set search path.
#define COMMAND_LINE_PARSING_AND_SINGLE_WORKER_DRTS_SETUP(extra_args_)        \
  ::boost::program_options::variables_map vm                                    \
    ( test::parse_command_line                                                \
        ( merge_with_cmdline_args (extra_args_)                               \
        , ::boost::program_options::options_description()                       \
        . add (test::options::source_directory())                             \
        . add (test::options::shared_directory())                             \
        . add (gspc::options::installation())                                 \
        . add (gspc::options::drts())                                         \
        . add (gspc::options::scoped_rifd())                                  \
        )                                                                     \
    );                                                                        \
                                                                              \
  test::scoped_nodefile_from_environment const nodefile                       \
    (test::shared_directory (vm), vm);                                        \
  gspc::set_application_search_path (vm, compile_pnet::app_search_path);      \
                                                                              \
  vm.notify();                                                                \
                                                                              \
  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}                  \
                                 , gspc::rifd::hostnames {vm}                 \
                                 , gspc::rifd::port {vm}                      \
                                 , vm                                         \
                                 );                                           \
  gspc::scoped_runtime_system const drts                                      \
    (vm, vm, "worker:1x1", rifds.entry_points());                             \
  gspc::client client (drts)

//! Given \a args_, the command line arguments, and \a expected_env_,
//! check that the worker has exactly that environment set.
#define RUN_AND_CHECK_RETURNED_ENVIRONMENT(args_, expected_env_)              \
  COMMAND_LINE_PARSING_AND_SINGLE_WORKER_DRTS_SETUP (args_);                  \
                                                                              \
  auto const result (client.put_and_run (compile_pnet::pnet, {}));            \
                                                                              \
  decltype (result) const expected                                            \
    {{"env", pnet::type::value::wrap (expected_env_)}};                       \
                                                                              \
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result)


BOOST_AUTO_TEST_CASE (specifying_nothing_leads_to_empty_env)
{
  std::vector<std::string> const args;
  std::set<std::string> const expected_env;

  RUN_AND_CHECK_RETURNED_ENVIRONMENT (args, expected_env);
}

// Counts chosen by random dice roll.
BOOST_DATA_TEST_CASE
  (copy_variable, std::vector<std::size_t> ({2, 3, 6, 8, 11, 13}), count)
{
  std::vector<random_env_kvpair> kvs (count);

  std::vector<std::string> args;
  std::set<std::string> expected_env;
  for (auto const& kv : kvs)
  {
    args.emplace_back ("--worker-env-copy-variable");
    args.emplace_back (kv.key);
    expected_env.emplace (kv.definition);
    fhg::util::syscall::setenv (kv.key.c_str(), kv.value.c_str(), 1);
  }
  FHG_UTIL_FINALLY
    ( [&]
      {
        for (auto const& kv : kvs)
        {
          fhg::util::syscall::unsetenv (kv.key.c_str());
        }
      }
    );

  RUN_AND_CHECK_RETURNED_ENVIRONMENT (args, expected_env);
}

BOOST_AUTO_TEST_CASE (specifying_current_leads_to_current_env)
{
  std::vector<std::string> const args
    {"--worker-env-copy-current"};
  std::set<std::string> expected_env;
  for (auto e (environ); *e; ++e)
  {
    expected_env.emplace (*e);
  }

  RUN_AND_CHECK_RETURNED_ENVIRONMENT (args, expected_env);
}

BOOST_AUTO_TEST_CASE (specifying_current_leads_to_current_env_ensure_something)
{
  random_env_kvpair const kvpair;
  fhg::util::syscall::setenv (kvpair.key.c_str(), kvpair.value.c_str(), 1);
  FHG_UTIL_FINALLY
    ( [&]
      {
        fhg::util::syscall::unsetenv (kvpair.key.c_str());
      }
    );

  std::vector<std::string> const args
    {"--worker-env-copy-current"};
  std::set<std::string> expected_env;
  for (auto e (environ); *e; ++e)
  {
    expected_env.emplace (*e);
  }

  RUN_AND_CHECK_RETURNED_ENVIRONMENT (args, expected_env);
}

BOOST_AUTO_TEST_CASE (copy_file_one_file)
{
  fhg::util::temporary_path const envfiles;
  fhg::util::temporary_file const envfile
    (envfiles / ::boost::filesystem::unique_path());

  std::vector<std::string> args;
  args.emplace_back ("--worker-env-copy-file");
  args.emplace_back (::boost::filesystem::path (envfile).string());

  // Count chosen by dice roll.
  std::vector<random_env_kvpair> kvs (17);

  std::set<std::string> expected_env;
  for (auto const& kv : kvs)
  {
    expected_env.emplace (kv.definition);
    std::ofstream (::boost::filesystem::path (envfile).string(), std::ios::app)
      << kv.definition << "\n";
  }

  RUN_AND_CHECK_RETURNED_ENVIRONMENT (args, expected_env);
}

BOOST_AUTO_TEST_CASE (copy_file_three_files)
{
  fhg::util::temporary_path const envfiles;
  fhg::util::temporary_file const envfile_0
    (envfiles / ::boost::filesystem::unique_path());
  fhg::util::temporary_file const envfile_1
    (envfiles / ::boost::filesystem::unique_path());
  fhg::util::temporary_file const envfile_2
    (envfiles / ::boost::filesystem::unique_path());
  std::array<::boost::filesystem::path, 3> const envfile_paths
    {envfile_0, envfile_1, envfile_2};

  std::vector<std::string> args;
  args.emplace_back ("--worker-env-copy-file");
  args.emplace_back (envfile_paths.at (0).string());
  args.emplace_back ("--worker-env-copy-file");
  args.emplace_back (envfile_paths.at (1).string());
  args.emplace_back ("--worker-env-copy-file");
  args.emplace_back (envfile_paths.at (2).string());

  // Count chosen by dice roll.
  std::vector<random_env_kvpair> kvs (58);

  std::set<std::string> expected_env;
  for (auto const& kv : kvs)
  {
    expected_env.emplace (kv.definition);
    std::ofstream
      ( envfile_paths.at
          (fhg::util::testing::random<std::size_t>{} (envfile_paths.size() - 1))
      . string()
      , std::ios::app
      ) << kv.definition << "\n";
  }

  RUN_AND_CHECK_RETURNED_ENVIRONMENT (args, expected_env);
}
