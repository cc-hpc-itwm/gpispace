// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <drts/scheduler_types.hpp>

#include <we/type/Activity.hpp>

namespace sdpa
{
  namespace daemon
  {
    gspc::scheduler::Type get_scheduler_type (we::type::Activity const&);
  }
}
