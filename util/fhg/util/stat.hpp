// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_STAT_HPP
#define _FHG_UTIL_STAT_HPP

#include <iosfwd>

#include <string>

namespace fhg
{
  namespace util
  {
    namespace stat
    {
      void out (std::ostream&);
      void inc (const std::string&);
      void start (const std::string&);
      void stop (const std::string&);
      void reset ();

#ifndef NSTATISTICS
#define FHG_UTIL_STAT_RESET()     ::fhg::util::stat::reset ()
#define FHG_UTIL_STAT_START(key)  ::fhg::util::stat::start (key)
#define FHG_UTIL_STAT_STOP(key)   ::fhg::util::stat::stop (key)
#define FHG_UTIL_STAT_INC(key)    ::fhg::util::stat::inc (key)
#define FHG_UTIL_STAT_OUT(stream) ::fhg::util::stat::out (stream)
#else
#define FHG_UTIL_STAT_RESET()
#define FHG_UTIL_STAT_START(key)
#define FHG_UTIL_STAT_STOP(key)
#define FHG_UTIL_STAT_INC(key)
#define FHG_UTIL_STAT_OUT(stream)
#endif
    }
  }
}

#endif
