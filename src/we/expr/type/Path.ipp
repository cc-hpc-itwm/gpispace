// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <utility>

namespace expr
{
  namespace type
  {
    template< typename... Args, typename>
      Path::Path (Args&&... args)
        : _particles (std::forward<Args> (args)...)
    {}
  }
}
