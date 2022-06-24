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

#include <preferences_and_multimodules/parameters.hpp>
#include <preferences_and_multimodules/parse_parameters_from_commandline.hpp>
#include <preferences_and_multimodules/execute.hpp>
#include <preferences_and_multimodules/workflow.hpp>
#include <preferences_and_multimodules/WorkflowResult.hpp>

#include <util-generic/print_exception.hpp>

#include <cstdlib>
#include <iostream>

int main (int argc, char** argv)
try
{
  // the command line options supplied by the user are parsed and stored
  auto const parameters
    ( preferences_and_multimodules::parse_parameters_from_commandline
        ( preferences_and_multimodules::execution::options()
        , preferences_and_multimodules::Workflow::options()
        , argc
        , argv
        )
    );


  // initialize the workflow
  preferences_and_multimodules::Workflow workflow (parameters);

  // execute the workflow and get back the result
  auto const result
    {preferences_and_multimodules::execute (workflow, parameters)};

  // evaluate the result
  workflow.process (result);

  return 0;
}
catch (...)
{
  std::cerr << "FAILURE: " << fhg::util::current_exception_printer() << std::endl;

  return EXIT_FAILURE;
}
