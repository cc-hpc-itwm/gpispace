#include "threadname.hpp"

#include <pthread.h>

namespace fhg
{
  namespace util
  {
#ifdef HAVE_PTHREAD_SETNAME

    int set_threadname (boost::thread & thrd, std::string const &name)
    {
      return pthread_setname_np ( thrd.native_handle ()
                                , name.c_str ()
                                );
    }

    int get_threadname (boost::thread & thrd, std::string &name)
    {
      char buf [128];

      const int ec (pthread_getname_np ( thrd.native_handle ()
                                       , buf
                                       , sizeof (buf)
                                       )
                   );

      buf [sizeof(buf) - 1] = '\0';

      if (!ec)
      {
        name = buf;
      }

      return ec;
    }

#else

    int set_threadname (boost::thread &, std::string const &)
    {
      return 0;
    }
    int get_threadname (boost::thread & , std::string &)
    {
      return 0;
    }

#endif
  }
}
