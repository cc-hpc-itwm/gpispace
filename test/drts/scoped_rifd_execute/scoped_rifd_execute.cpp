// Copyright (C) 2015-2016,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/util/boost/program_options/validators/existing_directory.hpp>
#include <gspc/util/boost/program_options/validators/existing_path.hpp>

#include <boost/program_options.hpp>

#include <filesystem>

#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>

#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

//! \todo remove
#include <gspc/util/print_exception.hpp>

BOOST_AUTO_TEST_CASE (scoped_rifd_from_command_line)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (gspc::testing::options::shared_directory());

  options_description.add_options()
    ( "test-binary"
    , ::boost::program_options::value
        <gspc::util::boost::program_options::existing_path>()->required()
    , "path to test binary"
    )
    ;

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  gspc::util::temporary_path const shared_directory
    (gspc::testing::shared_directory (vm) / "drts_scoped_rifd_execute");

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  std::filesystem::path const test_binary
    ( vm.at ("test-binary")
    . as<gspc::util::boost::program_options::existing_path>()
    );

  gspc::installation const installation (vm);

  gspc::scoped_rifds const scoped_rifds ( gspc::rifd::strategy {vm}
                                        , gspc::rifd::hostnames {vm}
                                        , gspc::rifd::port {vm}
                                        , installation
                                        );

  std::vector<std::string> const _hosts (scoped_rifds.hosts());
  std::unordered_set<std::string> const hosts {_hosts.begin(), _hosts.end()};

  { // execute works
    std::string const option_value (gspc::testing::random_identifier());
    auto const result ( scoped_rifds.execute
                        ( hosts
                        , test_binary
                        , {"--option", option_value}
                        )
                      );

    BOOST_REQUIRE_EQUAL (result.first.size(), hosts.size());
    BOOST_REQUIRE (result.second.empty());

    for (std::string const& host : hosts)
    {
      BOOST_REQUIRE (result.first.count (host));

      std::vector<std::string> const got (result.first.at (host));
      std::vector<std::string> const expected {option_value, host};

      BOOST_REQUIRE_EQUAL_COLLECTIONS
        (got.begin(), got.end(), expected.begin(), expected.end());
    }
  }

  { // execeptions are transported
    auto const result ( scoped_rifds.execute
                        ( hosts
                        , test_binary
                        )
                      );

    BOOST_REQUIRE (result.first.empty());
    BOOST_REQUIRE_EQUAL (result.second.size(), hosts.size());

    for (std::string const& host : hosts)
    {
      BOOST_REQUIRE (result.second.count (host));

      gspc::testing::require_exception
        ( [&result, &host]
          {
            std::rethrow_exception (result.second.at (host));
          }
        , std::logic_error ("the option '--option' is required but missing")
        );
    }
  }

  { // execeptions on unknown hosts
    auto const result ( scoped_rifds.execute
                        ( std::unordered_set<std::string> {""}
                        , {}
                        )
                      );

    BOOST_REQUIRE (result.first.empty());
    BOOST_REQUIRE_EQUAL (result.second.size(), 1);
    BOOST_REQUIRE (result.second.count (""));

    gspc::testing::require_exception
      ( [&result]
        {
          std::rethrow_exception (result.second.at (""));
        }
      , std::logic_error ("execute: unknown host ''")
      );
  }
}
