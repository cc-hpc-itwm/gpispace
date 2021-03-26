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

#include <boost/test/unit_test.hpp>

#include <iml/Rifs.hpp>

#include <boost/program_options.hpp>

#include <iml/testing/parse_command_line.hpp>
#include <iml/testing/set_nodefile_from_environment.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (scoped_rifd_from_command_line)
{
  boost::program_options::options_description options_description;

  options_description.add (iml::Rifs::options());

  boost::program_options::variables_map vm
    ( iml_test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  iml_test::set_nodefile_from_environment (vm);

  vm.notify();

  iml::Rifs {vm};
}
