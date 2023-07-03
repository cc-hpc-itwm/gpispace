// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <utility>

namespace sdpa
{
  namespace events
  {
    template<typename Event, typename... Args>
      std::string Codec::encode (Args&&... args) const
    {
      Event const e (std::forward<Args> (args)...);
      return encode (&e);
    }
  }
}
