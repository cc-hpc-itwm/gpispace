#pragma once

#include <aggregate_sum/Parameters.hpp>
#include <aggregate_sum/ValuesOnPorts.hpp>
#include <aggregate_sum/WorkflowResult.hpp>

namespace aggregate_sum
{
  class Workflow
  {
  public:
    static ParametersDescription options();

    Workflow (Parameters const& parameters);

    ValuesOnPorts inputs() const;

    int process (WorkflowResult const& result) const;

  private:
    int _N;
  };
}
