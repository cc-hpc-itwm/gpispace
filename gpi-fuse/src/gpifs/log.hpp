// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_LOG_HPP
#define FUSE_LOG_HPP 1

#ifndef NDEBUG
#include <iostream>
static std::ostream & logstream ()
{
  return std::cerr;
}
#define LOG(x) { logstream() << x << std::endl; }
#else
#define LOG(x)
#endif

#endif
