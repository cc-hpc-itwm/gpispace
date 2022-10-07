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

#include <iml/Rifs.hpp>
#include <iml/RuntimeSystem.hpp>

#include <boost/format.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/program_options.hpp>

#include <iml/testing/parse_command_line.hpp>
#include <iml/testing/set_nodefile_from_environment.hpp>
#include <iml/testing/virtual_memory_socket_name_for_localhost.hpp>

#include <util-generic/read_lines.hpp>
#include <util-generic/split.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/range/combine.hpp>

#include <algorithm>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (scoped_iml_rts_startup)
{
  ::boost::program_options::options_description options_description;

  options_description.add (iml::Rifs::options());
  options_description.add (iml::RuntimeSystem::options());

  ::boost::program_options::variables_map vm
    ( iml_test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  auto const nodefile (iml_test::set_nodefile_from_environment (vm));
  iml_test::set_iml_vmem_socket_path_for_localhost (vm);

  vm.notify();

  auto const hosts (fhg::util::read_lines (nodefile));

  iml::Rifs const rifs {vm};

  std::ostringstream info_output_stream;

  {
    iml::RuntimeSystem {rifs, vm, info_output_stream};
  }

  std::vector<std::string> expected_output (hosts.size() + 1);
  expected_output[0]
    = "I: starting VMEM on: (.+) with a timeout of [0-9]+ seconds";
  std::fill_n ( std::next (expected_output.begin())
              , hosts.size()
              , "terminating vmem on (.+) [0-9]+ [0-9]+: [0-9]+"
              );

  BOOST_TEST_CONTEXT (info_output_stream.str())
  {
    auto const output
      ( fhg::util::split<std::string, std::string, std::vector<std::string>>
          (info_output_stream.str(), '\n')
      );

    BOOST_REQUIRE_EQUAL (output.size(), expected_output.size());
    for (auto output_and_regex : ::boost::combine (output, expected_output))
    {
      auto const& line (::boost::get<0> (output_and_regex));
      auto const& expected_line (::boost::get<1> (output_and_regex));
      BOOST_REQUIRE_MESSAGE
        ( std::regex_match (line, std::regex {expected_line})
        , "line = '" << line << "', expected_line = '" << expected_line << "'"
        );
    }
  }
}
