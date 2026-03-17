// Copyright (C) 2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/testing/printer/we/type/value.hpp>
#include <gspc/we/type/value/wrap.hpp>

#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>

#include <gspc/util/finally.hpp>
#include <gspc/util/syscall.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/test/unit_test.hpp>

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
                  . add (gspc::testing::options::source_directory())
                  . add (gspc::testing::options::shared_directory())
                  . add (gspc::options::installation())
                  )
        . allow_unregistered()
        . run()
        , vm
        );
      vm.notify();

      app_search_path
        = gspc::testing::shared_directory (vm) / gspc::testing::unique_path();

      _make.emplace
        ( vm
        , "report_environment"
        , gspc::testing::source_directory (vm) / "worker_env"
        , std::filesystem::path {app_search_path.string()}
        );
      pnet = _make->pnet();
    }

    static std::filesystem::path app_search_path;
    static std::filesystem::path pnet;

    std::optional<gspc::testing::make_net_lib_install> _make;
  };

  std::filesystem::path compile_pnet::app_search_path = {};
  std::filesystem::path compile_pnet::pnet = {};

  using random_string = gspc::testing::random<std::string>;
}

// Note: random_env_kvpair must be outside the anonymous namespace
// to avoid -Wsubobject-linkage warnings when used with
// BOOST_DATA_TEST_CASE (which generates template instantiations).
struct random_env_kvpair
{
  static std::string random_key_that_does_not_interfere()
  {
    using random_string = gspc::testing::random<std::string>;
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
            // Used by gspc::testing::scoped_nodefile_from_environment
         || key == "GSPC_NODEFILE_FOR_TESTS"
          );
    return key;
  }

  random_env_kvpair()
    : key (random_key_that_does_not_interfere())
    , value ( gspc::testing::random<std::string>{}
                (gspc::testing::random<std::string>::except (std::string (1, '\0') + "\n"))
            )
    , definition (key + "=" + value)
  {}

  std::string const key;
  std::string const value;
  std::string const definition;
};

BOOST_GLOBAL_FIXTURE (compile_pnet);

//! - command line parsing `vm` with arguments from the actual command
//!   line, merged with the vector<string> \a extra_args_
//! - `shared_directory` + `lib_install_directory` + nodefile
//! - rifds + 1 worker "worker" per node
//! - `client`
//! - use the global pnet compilation to set search path.
#define COMMAND_LINE_PARSING_AND_SINGLE_WORKER_DRTS_SETUP(extra_args_)        \
  ::boost::program_options::variables_map vm                                    \
    ( gspc::testing::parse_command_line                                                \
        ( merge_with_cmdline_args (extra_args_)                               \
        , ::boost::program_options::options_description()                       \
        . add (gspc::testing::options::source_directory())                             \
        . add (gspc::testing::options::shared_directory())                             \
        . add (gspc::options::installation())                                 \
        . add (gspc::options::drts())                                         \
        . add (gspc::options::scoped_rifd())                                  \
        )                                                                     \
    );                                                                        \
                                                                              \
  gspc::testing::scoped_nodefile_from_environment const nodefile                       \
    (gspc::testing::shared_directory (vm), vm);                                        \
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
    {{"env", gspc::pnet::type::value::wrap (expected_env_)}};                       \
                                                                              \
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result)


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
    gspc::util::syscall::setenv (kv.key.c_str(), kv.value.c_str(), 1);
  }
  FHG_UTIL_FINALLY
    ( [&]
      {
        for (auto const& kv : kvs)
        {
          gspc::util::syscall::unsetenv (kv.key.c_str());
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
  gspc::util::syscall::setenv (kvpair.key.c_str(), kvpair.value.c_str(), 1);
  FHG_UTIL_FINALLY
    ( [&]
      {
        gspc::util::syscall::unsetenv (kvpair.key.c_str());
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
  gspc::testing::temporary_path const envfiles;
  gspc::util::temporary_file const envfile
    ( std::filesystem::path {envfiles}
    / gspc::testing::unique_path()
    );

  std::vector<std::string> args;
  args.emplace_back ("--worker-env-copy-file");
  args.emplace_back (std::filesystem::path (envfile).string());

  // Count chosen by dice roll.
  std::vector<random_env_kvpair> kvs (17);

  std::set<std::string> expected_env;
  for (auto const& kv : kvs)
  {
    expected_env.emplace (kv.definition);
    std::ofstream (std::filesystem::path (envfile), std::ios::app)
      << kv.definition << "\n";
  }

  RUN_AND_CHECK_RETURNED_ENVIRONMENT (args, expected_env);
}

BOOST_AUTO_TEST_CASE (copy_file_three_files)
{
  gspc::testing::temporary_path const envfiles;
  gspc::util::temporary_file const envfile_0
    ( std::filesystem::path {envfiles}
    / gspc::testing::unique_path()
    );
  gspc::util::temporary_file const envfile_1
    ( std::filesystem::path {envfiles}
    / gspc::testing::unique_path()
    );
  gspc::util::temporary_file const envfile_2
    ( std::filesystem::path {envfiles}
    / gspc::testing::unique_path()
    );
  std::array<std::filesystem::path, 3> const envfile_paths
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
          (gspc::testing::random<std::size_t>{} (envfile_paths.size() - 1))
      . string()
      , std::ios::app
      ) << kv.definition << "\n";
  }

  RUN_AND_CHECK_RETURNED_ENVIRONMENT (args, expected_env);
}
