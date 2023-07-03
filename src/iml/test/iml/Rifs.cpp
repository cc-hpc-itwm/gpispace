// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <iml/Rifs.hpp>

#include <boost/program_options.hpp>

#include <iml/testing/parse_command_line.hpp>
#include <iml/testing/set_nodefile_from_environment.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (scoped_rifd_from_command_line)
{
  ::boost::program_options::options_description options_description;

  options_description.add (iml::Rifs::options());

  ::boost::program_options::variables_map vm
    ( iml_test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  iml_test::set_nodefile_from_environment (vm);

  vm.notify();

  iml::Rifs {vm};
}
