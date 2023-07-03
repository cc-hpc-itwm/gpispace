// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace fhg
{
  namespace iml
  {
    namespace util
    {
      void system_with_blocked_SIGCHLD (std::string const&);
    }
  }
}
