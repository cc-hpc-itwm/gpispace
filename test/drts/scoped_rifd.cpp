// Copyright (C) 2015-2016,2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <boost/program_options.hpp>

#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>

#include <gspc/util/temporary_path.hpp>

BOOST_AUTO_TEST_CASE (scoped_rifd_from_command_line)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (gspc::testing::options::shared_directory());

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  gspc::util::temporary_path const shared_directory
    (gspc::testing::shared_directory (vm) / "drts_scoped_rifd");

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::scoped_rifds const scoped_rifds ( gspc::rifd::strategy {vm}
                                        , gspc::rifd::hostnames {vm}
                                        , gspc::rifd::port {vm}
                                        , installation
                                        );
}
