// {petry,mirko.rahn}@itwm.fraunhofer.de

#ifndef _FHG_UTIL_READ_HPP
#define _FHG_UTIL_READ_HPP 1

#include <sstream>
#include <stdexcept>
#include <ios>
#include <typeinfo>

namespace fhg
{
  namespace util
  {
    template<typename T>
    inline T read (const std::string & showed)
    {
      T x;
      std::istringstream i (showed);
      i >> x;
      if (i.fail ())
      {
        throw std::invalid_argument
          ("fhg::util::read (" + showed + "): could not be read as " + typeid(T).name ());
      }
      return x;
    }
  }
}

#endif
