// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
