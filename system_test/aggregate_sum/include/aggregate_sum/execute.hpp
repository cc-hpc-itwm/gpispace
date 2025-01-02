// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <aggregate_sum/Parameters.hpp>
#include <aggregate_sum/WorkflowResult.hpp>

namespace aggregate_sum
{
  class Workflow;

  namespace execution
  {
    ParametersDescription options();
  }

  WorkflowResult execute (Parameters, Workflow const&);
}
