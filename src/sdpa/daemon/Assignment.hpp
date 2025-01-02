// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
