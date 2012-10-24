#include "threadname.hpp"

#include <pthread.h>

namespace fhg
{
  namespace util
  {
    int set_threadname (boost::thread & thrd, std::string const &name)
    {
#ifdef HAVE_PTHREAD_SETNAME
      return pthread_setname_np ( thrd.native_handle ()
                                , name.c_str ()
                                );
#else
      return 0;
#endif
    }

    int get_threadname (boost::thread & thrd, std::string &name)
    {
      int rc = 0;

#ifdef HAVE_PTHREAD_SETNAME
      char buf [128];

      rc = pthread_getname_np ( thrd.native_handle ()
                              , buf
                              , sizeof (buf)
                              );
      buf [sizeof(buf) - 1] = 0;
      if (rc == 0)
        name = buf;
#endif

      return rc;
    }
  }
}
