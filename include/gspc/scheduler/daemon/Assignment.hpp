// Copyright (C) 2022-2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/daemon/Implementation.hpp>
#include <gspc/scheduler/daemon/WorkerSet.hpp>

#include <set>
#include <string>


  namespace gspc::scheduler::daemon
  {
    struct Assignment
    {
      WorkerSet _workers;
      Implementation _implementation;
      double _total_transfer_cost = 0.0;
      std::set<std::string> _worker_class;
    };
  }
