// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

    Workflow (Parameters const&);

    ValuesOnPorts inputs() const;

    int process (WorkflowResult const&) const;

  private:
    int _N;
  };
}
