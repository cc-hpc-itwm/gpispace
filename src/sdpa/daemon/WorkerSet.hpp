// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/types.hpp>

#include <set>

namespace sdpa
{
  namespace daemon
  {
    using WorkerSet = std::set<worker_id_t>;
  }
}
