// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      //! Execute the given function after one Qt event loop cycle.
      void do_after_event_loop (std::function<void()>);
    }
  }
}
