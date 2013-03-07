#ifndef FHG_UTIL_THREAD_SELECT_HPP
#define FHG_UTIL_THREAD_SELECT_HPP

#include <stdlib.h>

namespace fhg
{
  namespace thread
  {
#define FHG_THREAD_SELECT_MS    1
#define FHG_THREAD_SELECT_TEST  0
#define FHG_THREAD_SELECT_WAIT -1

    class pollable;
    struct poll_item_t
    {
      pollable      *to_poll;
      int                 fd;
      short           events;
      short          revents;
    };

    int poll (poll_item_t *, size_t nitems, long timeout);
  }
}

#endif
