// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace we::type
{
  constexpr auto operator==
    ( TokenOnPort const& lhs
    , TokenOnPort const& rhs
    ) -> bool
  {
    constexpr auto essence
      { [] (auto const& x)
        {
          return std::tie (x._port_id, x._token);
        }
      };

    return essence (lhs) == essence (rhs);
  }
}
