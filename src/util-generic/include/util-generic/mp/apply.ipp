// Copyright (C) 2025 Fraunhofer ITWM
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
        template< template<typename> class Operation
                , template<typename...> class Sequence
                , typename... Types
                >
          struct apply<Operation, Sequence<Types...>>
        {
          using type = Sequence<Operation<Types>...>;
        };
      }
    }
  }
}
