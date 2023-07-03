// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <preferences_and_multimodules/parameters.hpp>
#include <preferences_and_multimodules/WorkflowResult.hpp>

#include <we/type/value.hpp>

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace preferences_and_multimodules
{
  unsigned int const NUM_PREFERENCES {3};

  class Workflow
  {
  public:
    static ParametersDescription options();

    Workflow (Parameters const&);

    long get_num_tasks() const;
    std::size_t get_num_nodes () const;
    void set_num_nodes (std::size_t);
    std::array<std::string, NUM_PREFERENCES> get_preferences() const;
    std::vector<unsigned long> get_num_workers_per_target() const;

    void process (WorkflowResult);

  private:
    long _num_tasks;
    std::array<std::string, NUM_PREFERENCES> const _preferences
      {"FPGA", "GPU", "CPU"};
    std::vector<unsigned long> _num_workers_per_target;
    std::size_t _num_nodes;
  };
}
