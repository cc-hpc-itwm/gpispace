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
