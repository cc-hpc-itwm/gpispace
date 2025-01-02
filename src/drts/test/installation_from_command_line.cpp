// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <boost/program_options.hpp>

BOOST_AUTO_TEST_CASE (installation_from_command_line)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::options::installation());

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser
      ( ::boost::unit_test::framework::master_test_suite().argc
      , ::boost::unit_test::framework::master_test_suite().argv
      ).options (options_description).run()
    , vm
    );

  vm.notify();

  gspc::installation const installation (vm);
}
