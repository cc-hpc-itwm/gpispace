#include "util.hpp"

#ifdef __linux__
#  ifndef _GNU_SOURCE
#    define _GNU_SOURCE
#  endif
#  include <unistd.h>
#  include <sys/syscall.h>
#else
#  include <boost/thread.hpp>
#  include <boost/lexical_cast.hpp>
#endif

namespace fhg
{
  namespace log
  {
    unsigned int gettid()
    {
#ifdef __linux__
      return (unsigned int)(syscall(SYS_gettid));
#else
      return boost::lexical_cast<unsigned int>(boost::this_thread::get_id());
#endif
    }
  }
}
