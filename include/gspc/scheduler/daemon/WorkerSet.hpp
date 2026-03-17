// Copyright (C) 2021-2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/types.hpp>

#include <set>


  namespace gspc::scheduler::daemon
  {
    using WorkerSet = std::set<worker_id_t>;
  }
