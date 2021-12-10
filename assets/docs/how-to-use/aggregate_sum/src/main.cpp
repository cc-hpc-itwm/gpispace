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
