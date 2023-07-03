// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <preferences_and_multimodules/parameters.hpp>
#include <preferences_and_multimodules/workflow.hpp>

namespace preferences_and_multimodules
{
  namespace execution
  {
    ParametersDescription options();
  }

  WorkflowResult execute (Workflow, Parameters);
}
