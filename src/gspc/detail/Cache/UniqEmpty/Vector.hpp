#pragma once

#include <vector>

namespace gspc
{
  namespace detail
  {
    namespace Cache
    {
      namespace UniqEmpty
      {
        template<typename ID>
          struct Vector
        {
          bool empty() const;
          void push (ID);
          ID pop();

        private:
          std::vector<ID> _;
        };
      }
    }
  }
}

#include <gspc/detail/Cache/UniqEmpty/Vector.ipp>
