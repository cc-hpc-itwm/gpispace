#pragma once

#include <gspc/detail/Cache/Entry.fwd.hpp>
#include <gspc/detail/References.hpp>

namespace gspc
{
  namespace detail
  {
    namespace Cache
    {
      template<typename ID, typename Counter>
        struct Entry
      {
        Entry (ID);

        ID id() const;

        bool remembered() const;
        void remember();

        bool in_use() const;
        bool is_first_use();
        bool is_last_use();

      private:
        ID _id;
        bool _remembered;
        References<Counter> _references;
      };
    }
  }
}

#include <gspc/detail/Cache/Entry.ipp>
