#ifndef FHG_UTIL_THREAD_ASYNC_HPP
#define FHG_UTIL_THREAD_ASYNC_HPP

#include <fhg/util/thread/pool.hpp>

namespace fhg
{
  namespace thread
  {
    template <typename Fun>
    void async (Fun f)
    {
      global_pool ().execute (f);
    }

    template <typename Fun, typename Callback>
    void async (Fun f, Callback cb)
    {
      global_pool ().execute (f, cb);
    }
  }
}

#endif
