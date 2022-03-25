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

#include <aggregate_sum/parse_parameters_from_commandline.hpp>
#include <aggregate_sum/execute.hpp>
#include <aggregate_sum/Parameters.hpp>
#include <aggregate_sum/Workflow.hpp>

#include <util-generic/print_exception.hpp>

#include <cstdlib>
#include <iostream>

int main (int argc, char** argv)
try
{
  // (1) loading configuration options
  auto const parameters = aggregate_sum::parse_parameters_from_commandline
    (aggregate_sum::execution::options(),
     aggregate_sum::Workflow::options(),
     argc,
     argv
    );

  // (2) initializing a workflow
  aggregate_sum::Workflow const workflow (parameters);

  // (3) executing the workflow
  auto const results = aggregate_sum::execute (parameters, workflow);

  // (4) evaluating the workflow result
  return workflow.process (results);
}
catch (...)
{
  std::cerr << "FAILURE: " << fhg::util::current_exception_printer() << std::endl;

  return EXIT_FAILURE;
}
