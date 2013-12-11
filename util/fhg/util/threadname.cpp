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

#else

    int set_threadname (boost::thread &, std::string const &)
    {
      return 0;
    }

#endif
  }
}
