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

#include <sdpa/daemon/Implementation.hpp>
#include <sdpa/daemon/WorkerSet.hpp>

#include <set>
#include <string>

namespace sdpa
{
  namespace daemon
  {
    struct Assignment
    {
      WorkerSet _workers;
      Implementation _implementation;
      double _total_transfer_cost = 0.0;
      std::set<std::string> _worker_class;
    };
  }
}
