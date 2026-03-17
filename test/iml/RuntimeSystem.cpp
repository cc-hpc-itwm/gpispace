// Copyright (C) 2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/iml/Rifs.hpp>
#include <gspc/iml/RuntimeSystem.hpp>

#include <boost/program_options.hpp>

#include <gspc/testing/iml/parse_command_line.hpp>
#include <gspc/testing/iml/set_nodefile_from_environment.hpp>
#include <gspc/testing/iml/virtual_memory_socket_name_for_localhost.hpp>

#include <gspc/util/read_lines.hpp>
#include <gspc/util/split.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/optional.hpp>

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

  options_description.add (gspc::iml::Rifs::options());
  options_description.add (gspc::iml::RuntimeSystem::options());

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

  auto const hosts (gspc::util::read_lines (nodefile));

  gspc::iml::Rifs const rifs {vm};

  std::ostringstream info_output_stream;

  {
    gspc::iml::RuntimeSystem {rifs, vm, info_output_stream};
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
      ( gspc::util::split<std::string, std::string, std::vector<std::string>>
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
