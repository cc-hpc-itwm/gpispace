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

#include <aggregate_sum/Workflow.hpp>

#include <iostream>

namespace aggregate_sum
{
  ParametersDescription Workflow::options()
  {
    namespace po = boost::program_options;

    ParametersDescription workflow_opts ("Workflow");
    workflow_opts.add_options()("N", po::value<int>()->required());

    return workflow_opts;
  }

  Workflow::Workflow (Parameters const& args)
    : _N (args.at ("N").as<int>())
  {}

  ValuesOnPorts Workflow::inputs() const
  {
    ValuesOnPorts::Map values_on_ports;

    for (int i = 1; i <= _N; ++i)
    {
      values_on_ports.emplace ("values", i);
    }

    return values_on_ports;
  }

  int Workflow::process (WorkflowResult const& results) const
  {
    auto const& sum = results.get<int> ("sum");

    std::cout << "Aggregate Sum: " << sum << std::endl;

    return sum == _N * (_N + 1) / 2 ? EXIT_SUCCESS : EXIT_FAILURE;
  }
}
