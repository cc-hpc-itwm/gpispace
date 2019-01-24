#pragma once

#include <gspc/detail/StencilCache/InputEntry.fwd.hpp>

#include <gspc/detail/References.hpp>

#include <unordered_set>

namespace gspc
{
  namespace detail
  {
    namespace StencilCache
    {
      template<typename Output, typename Counter>
        struct InputEntry
      {
        void increment();

        bool free();
        std::unordered_set<Output> const& waiting() const;
        void triggers (Output);
        void prepared();

      private:
        References<Counter> _references;
        std::unordered_set<Output> _waiting;
      };
    }
  }
}

#include <gspc/detail/StencilCache/InputEntry.ipp>
