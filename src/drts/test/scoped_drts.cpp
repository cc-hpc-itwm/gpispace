// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <boost/program_options.hpp>

#include <testing/certificates_data.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>

#include <util-generic/read_lines.hpp>
#include <util-generic/split.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/data/test_case.hpp>

#include <fmt/core.h>
#include <regex>
#include <sstream>
#include <vector>

BOOST_DATA_TEST_CASE
  ( scoped_drts_empty_topology
  , certificates_data
  , certificates
  )
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (test::options::shared_directory());

  ::boost::program_options::variables_map vm
    ( test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "drts_scoped_drts");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::scoped_rifds const scoped_rifds ( gspc::rifd::strategy {vm}
                                        , gspc::rifd::hostnames {vm}
                                        , gspc::rifd::port {vm}
                                        , installation
                                        );

  gspc::scoped_runtime_system const drts
    (vm, installation, "", scoped_rifds.entry_points(), std::cerr, certificates);
}

BOOST_DATA_TEST_CASE
  ( no_worker_started_on_parent
  , certificates_data
  , certificates
  )
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (test::options::shared_directory());

  ::boost::program_options::variables_map vm
    ( test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "drts_scoped_drts");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  std::vector<std::string> const hosts
    (fhg::util::read_lines (nodefile_from_environment.path()));

  gspc::scoped_rifd const parent
    ( gspc::rifd::strategy {vm}
    , gspc::rifd::hostname {hosts.front()}
    , gspc::rifd::port {vm}
    , installation
    );

  std::ostringstream info_output_stream;

  {
    gspc::scoped_runtime_system const drts
      ( vm
      , installation
      , "worker:1"
      , ::boost::none
      , parent.entry_point()
      , info_output_stream
      , certificates
      );
  }

  {
    std::vector<std::string> const info_output
      ( fhg::util::split<std::string, std::string, std::vector<std::string>>
          (info_output_stream.str(), '\n')
      );

    BOOST_TEST_CONTEXT (info_output_stream.str())
    BOOST_REQUIRE_EQUAL (info_output.size(), 6);

    std::string const entry_point_parent
      ([&info_output, &hosts]()
       {
         std::smatch match;

         BOOST_REQUIRE
           ( std::regex_match
             ( info_output[0]
             , match
             , std::regex
               ("I: starting base sdpa components on ((.+) [0-9]+ [0-9]+)...")
             )
           );
         BOOST_REQUIRE_EQUAL (match.size(), 3);
         BOOST_REQUIRE_EQUAL (match[2].str(), hosts.front());

         return match[1].str();
       }()
      );

    BOOST_REQUIRE
      ( std::regex_match
        ( info_output[1]
        , std::regex {"I: starting top level gspc logging demultiplexer on .*"}
        )
      );
    BOOST_REQUIRE
      ( std::regex_match
        ( info_output[2]
        , std::regex {"   => accepting registration on 'TCP: <<.*>>'"}
        )
      );

    BOOST_REQUIRE_EQUAL
      ( info_output[3]
      , fmt::format
         ( "I: starting agent: agent-{0}-0 on rif entry point {0}"
         , entry_point_parent
         )
      );

    BOOST_REQUIRE
      ( std::regex_match
        ( info_output[4]
        , std::regex
          {fmt::format ("terminating agent on {}: [0-9]+", entry_point_parent)}
        )
      );

    BOOST_REQUIRE
      ( std::regex_match
        ( info_output[5]
        , std::regex
            { fmt::format
              ( "terminating logging-demultiplexer on {}: [0-9]+"
              , entry_point_parent
              )
            }
        )
      );
  }
}

BOOST_DATA_TEST_CASE
  ( workers_are_started_on_child
  , certificates_data
  , certificates
  )
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (test::options::shared_directory());

  ::boost::program_options::variables_map vm
    ( test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "drts_scoped_drts");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  std::vector<std::string> const hosts
    (fhg::util::read_lines (nodefile_from_environment.path()));

  gspc::scoped_rifd const parent
    ( gspc::rifd::strategy {vm}
    , gspc::rifd::hostname {hosts.front()}
    , gspc::rifd::port {vm}
    , installation
    );
  gspc::scoped_rifds const scoped_rifds
    ( gspc::rifd::strategy {vm}
    , gspc::rifd::hostnames {{hosts.begin(), std::next (hosts.begin())}}
    , gspc::rifd::port {vm}
    , installation
    );

  std::ostringstream info_output_stream;
  std::string const worker ("WORKER");

  {
    gspc::scoped_runtime_system const drts
      ( vm
      , installation
      , worker + ":1"
      , scoped_rifds.entry_points()
      , parent.entry_point()
      , info_output_stream
      , certificates
      );
  }

  {
    std::vector<std::string> const info_output
      ( fhg::util::split<std::string, std::string, std::vector<std::string>>
          (info_output_stream.str(), '\n')
      );

    BOOST_TEST_CONTEXT (info_output_stream.str())
    BOOST_REQUIRE_EQUAL (info_output.size(), 8);

    std::string const entry_point_parent
      ([&info_output, &hosts]()
       {
         std::smatch match;

         BOOST_REQUIRE
           ( std::regex_match
             ( info_output[0]
             , match
             , std::regex
               ("I: starting base sdpa components on ((.+) [0-9]+ [0-9]+)...")
             )
           );
         BOOST_REQUIRE_EQUAL (match.size(), 3);
         BOOST_REQUIRE_EQUAL (match[2].str(), hosts.front());

         return match[1].str();
       }()
      );

    BOOST_REQUIRE
      ( std::regex_match
        ( info_output[1]
        , std::regex {"I: starting top level gspc logging demultiplexer on .*"}
        )
      );
    BOOST_REQUIRE
      ( std::regex_match
        ( info_output[2]
        , std::regex {"   => accepting registration on 'TCP: <<.*>>'"}
        )
      );

    BOOST_REQUIRE_EQUAL
      ( info_output[3]
      , fmt::format
          ( "I: starting agent: agent-{0}-0 on rif entry point {0}"
          , entry_point_parent
          )
      );

    std::string const entry_point_worker
      ([&info_output, &hosts, &entry_point_parent, &worker]()
       {
         std::smatch match;

         BOOST_REQUIRE
           ( std::regex_match
             ( info_output[4]
             , match
             , std::regex
               { fmt::format
                 ( "I: starting {1} workers"
                   " \\(parent agent-{0}-0, 1/host, unlimited, 0 SHM\\)"
                   " with parent agent-{0}-0"
                   " on rif entry point ((.+) [0-9]+ [0-9]+)"
                 , entry_point_parent
                 , worker
                 )
               }
             )
           );
         BOOST_REQUIRE_EQUAL (match.size(), 3);
         BOOST_REQUIRE_EQUAL (match[2].str(), hosts.front());

         return match[1].str();
       }()
      );

    BOOST_REQUIRE
      ( std::regex_match
        ( info_output[5]
        , std::regex
          { fmt::format
              ( "terminating drts-kernel on {}: [0-9]+"
              , entry_point_worker
              )
          }
        )
      );

    BOOST_REQUIRE
      ( std::regex_match
        ( info_output[6]
        , std::regex
          { fmt::format ("terminating agent on {}: [0-9]+", entry_point_parent)
          }
        )
      );

    BOOST_REQUIRE
      ( std::regex_match
        ( info_output[7]
        , std::regex
          { fmt::format
            ( "terminating logging-demultiplexer on {}: [0-9]+"
            , entry_point_parent
            )
          }
        )
      );
  }
}
