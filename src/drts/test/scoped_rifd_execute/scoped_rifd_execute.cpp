// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <util-generic/boost/program_options/validators/existing_directory.hpp>
#include <util-generic/boost/program_options/validators/existing_path.hpp>

#include <boost/program_options.hpp>

#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

//! \todo remove
#include <util-generic/print_exception.hpp>

BOOST_AUTO_TEST_CASE (scoped_rifd_from_command_line)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (test::options::shared_directory());

  options_description.add_options()
    ( "test-binary"
    , ::boost::program_options::value
        <fhg::util::boost::program_options::existing_path>()->required()
    , "path to test binary"
    )
    ;

  ::boost::program_options::variables_map vm
    ( test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "drts_scoped_rifd_execute");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  ::boost::filesystem::path const test_binary
    ( vm.at ("test-binary")
    . as<fhg::util::boost::program_options::existing_path>()
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
    std::string const option_value (fhg::util::testing::random_identifier());
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
    std::string const option_value (fhg::util::testing::random_identifier());
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

      fhg::util::testing::require_exception
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

    fhg::util::testing::require_exception
      ( [&result]
        {
          std::rethrow_exception (result.second.at (""));
        }
      , std::logic_error ("execute: unknown host ''")
      );
  }
}
