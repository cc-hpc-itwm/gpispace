// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace gspc::we::type
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
