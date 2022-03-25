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
