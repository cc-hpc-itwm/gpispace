#pragma once

#include <gspc/detail/Cache/UniqUnused/UnorderedSet.hpp>

namespace gspc
{
  namespace detail
  {
    namespace Cache
    {
      namespace UniqEmpty
      {
        template<typename ID>
          using UnorderedSet = UniqUnused::UnorderedSet<ID>;
      }
    }
  }
}
