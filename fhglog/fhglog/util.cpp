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
#ifdef __APPLE__
#include <crt_externs.h> // _NSGetEnviron
#endif

#include <fhg/util/split.hpp>

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

    environment_t get_environment_variables()
    {
      environment_t env;
#ifdef __APPLE__
      char ** env_p = *_NSGetEnviron();
#else
      char ** env_p = environ;
#endif
      while (env_p != NULL && (*env_p != NULL))
      {
        env.push_back(fhg::util::split_string(*env_p, "="));
        ++env_p;
      }
      return env;
    }
  }
}
