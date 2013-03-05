#include <cerrno>
#include <boost/thread.hpp>
#include <fhg/util/thread/select.hpp>

namespace fhg
{
  namespace thread
  {
    static bool s_select_on_item (select_item_t *item)
    {
      item->revents = 0;

      if (item->events & fhg::thread::POLL_IN)
      {
        if (item->selectable->is_ready_for (fhg::thread::POLL_IN))
        {
          item->revents |= fhg::thread::POLL_IN;
        }
      }

      if (item->events & fhg::thread::POLL_OUT)
      {
        if (item->selectable->is_ready_for (fhg::thread::POLL_OUT))
        {
          item->revents |= fhg::thread::POLL_OUT;
        }
      }

      return item->revents != 0;
    }

    int select (select_item_t *items, size_t nitems, long _ms)
    {
      boost::posix_time::time_duration duration =
        boost::posix_time::milliseconds (_ms);
      boost::system_time const timeout = boost::get_system_time() + duration;

      while (true)
      {
        int nready = 0;

        for (size_t i = 0 ; i < nitems ; ++i)
        {
          if (s_select_on_item (&items [i]))
            ++nready;
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
