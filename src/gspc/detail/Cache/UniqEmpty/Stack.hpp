#pragma once

#include <stack>

namespace gspc
{
  namespace detail
  {
    namespace Cache
    {
      namespace UniqEmpty
      {
        template<typename ID>
          struct Stack
        {
          bool empty() const;
          void push (ID);
          ID pop();

        private:
          std::stack<ID> _;
        };
      }
    }
  }
}

#include <gspc/detail/Cache/UniqEmpty/Stack.ipp>
