// Copyright (C) 2019,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <utility>


  namespace gspc::scheduler::events
  {
    template<typename Event, typename... Args>
      std::string Codec::encode (Args&&... args) const
    {
      Event const e (std::forward<Args> (args)...);
      return encode (&e);
    }
  }
