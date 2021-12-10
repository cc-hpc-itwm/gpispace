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
