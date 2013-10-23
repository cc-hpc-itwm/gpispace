#include "safe_run_io_service.hpp"

#include <errno.h>

namespace gspc
{
  namespace net
  {
    int safe_run_io_service (boost::asio::io_service *s) try
    {
      if (0 == s)
      {
        return -EINVAL;
      }
      else
      {
        s->run ();
      }
      return 0;
    }
    catch (...)
    {
      return -EINTR;
    }
  }
}
