#include "drts_callbacks.h"

#include <list>
#include <utility>

#include <boost/thread.hpp>

typedef std::pair<DrtsCancelHandler, void*> handler_t;
typedef std::list<handler_t> handler_list_t;

typedef boost::mutex mutex_type;
typedef boost::condition_variable condition_type;
typedef boost::unique_lock<mutex_type> lock_type;

static handler_list_t s_handler_list;
static mutex_type s_handler_list_mtx;

int drts_on_cancel_add (DrtsCancelHandler h, void *data)
{
  lock_type lck (s_handler_list_mtx);
  s_handler_list.push_back (std::make_pair (h, data));
  return 0;
}

void drts_on_cancel_clear ()
{
  lock_type lck (s_handler_list_mtx);
  s_handler_list.clear ();
}

int drts_on_cancel ()
{
  int ec = 0;

  handler_list_t to_call;
  {
    lock_type lck (s_handler_list_mtx);
    to_call.swap (s_handler_list);
  }

  while (not to_call.empty ())
  {
    handler_t handler = to_call.back ();
    to_call.pop_back ();
    try
    {
      handler.first (handler.second);
    }
    catch (...)
    {
      ec = -1;
    }
  }

  return ec;
}
