#ifndef FHG_UTIL_THREAD_NAME_HPP
#define FHG_UTIL_THREAD_NAME_HPP

#include <boost/thread.hpp>
#include <string>

namespace fhg
{
  namespace util
  {
    extern int set_threadname (boost::thread & thrd, std::string const &name);
    extern int get_threadname (boost::thread & thrd, std::string &name);
  }
}

#endif
