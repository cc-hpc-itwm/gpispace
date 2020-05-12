#pragma once

#include <unordered_set>

namespace gspc
{
  namespace detail
  {
    namespace Cache
    {
      namespace UniqUnused
      {
        template<typename T>
          struct UnorderedSet
        {
          bool empty() const;
          void push (T);
          T pop();
          void erase (T);

        private:
          std::unordered_set<T> _;
        };
      }
    }
  }
}

#include <gspc/detail/Cache/UniqUnused/UnorderedSet.ipp>
