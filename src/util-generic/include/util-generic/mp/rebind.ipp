// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace fhg
{
  namespace util
  {
    namespace mp
    {
      namespace detail
      {
        template < template<typename...> class To
                 , template<typename...> class From
                 , typename... T
                 >
          struct rebind<To, From<T...>>
        {
          using type = To<T...>;
        };
      }
    }
  }
}
