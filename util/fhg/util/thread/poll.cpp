#include <cerrno>
#include <boost/thread.hpp>
#include <fhg/util/thread/pollable.hpp>
#include <fhg/util/thread/poll.hpp>

namespace fhg
{
  namespace thread
  {
    static bool s_select_on_item (poll_item_t *item)
    {
      item->revents = 0;
      item->revents = (item->events & item->to_poll->poll ());

      return item->revents != 0;
    }

    int poll (poll_item_t *items, size_t nitems, long _ms)
    {
      boost::system_time const timeout
        = boost::get_system_time()
        + boost::posix_time::milliseconds (_ms);

      while (true)
      {
        int nready = 0;

        for (size_t i = 0 ; i < nitems ; ++i)
        {
          if (items [i].to_poll)
          {
            if (s_select_on_item (&items [i]))
            {
              ++nready;
            }
          }
        }

        if (nready || 0 == _ms)
          return nready;

        if (_ms != -1 && timeout < boost::get_system_time ())
          return 0;

        if (-1 == usleep (std::min (250L, _ms)))
        {
          return -1;
        }
      }

      return 0;
    }
  }
}
