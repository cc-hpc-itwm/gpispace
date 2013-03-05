#ifndef FHG_UTIL_THREAD_SELECT_HPP
#define FHG_UTIL_THREAD_SELECT_HPP

#include <stdlib.h>
#include <fhg/util/thread/selectable.hpp>

namespace fhg
{
  namespace thread
  {
#define FHG_THREAD_SELECT_MS    1
#define FHG_THREAD_SELECT_TEST  0
#define FHG_THREAD_SELECT_WAIT -1

    struct select_item_t
    {
      selectable *selectable;
      short           events;
      short          revents;
    };

    int select (select_item_t *, size_t nitems, long timeout);
  }
}

#endif
