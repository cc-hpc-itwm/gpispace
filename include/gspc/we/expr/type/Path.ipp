// Copyright (C) 2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <utility>


  namespace gspc::we::expr::type
  {
    template< typename... Args, typename>
      Path::Path (Args&&... args)
        : _particles (std::forward<Args> (args)...)
    {}
  }
