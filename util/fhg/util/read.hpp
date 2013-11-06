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
    inline T read ( const std::string & showed
                  , std::ios_base::fmtflags flags=std::ios_base::fmtflags ()
                  )
    {
      T x;
      std::istringstream i (showed);
      i.flags (flags);
      i >> x;
      if (i.fail ())
      {
        throw std::invalid_argument
          ("fhg::util::read (" + showed + "): could not be read as " + typeid(T).name ());
      }
      return x;
    }

    template<>
    inline std::string read ( const std::string & s
                            , std::ios_base::fmtflags flgs
                            )
    {
      return s;
    }

    template <typename T>
    struct reader
    {
      inline static T read ( std::string const & s
                           , std::ios_base::fmtflags flags
                           )
      {
        return fhg::util::read<T>(s, flags);
      }

      inline static T read (std::string const & s)
      {
        return fhg::util::read<T>(s, std::ios_base::fmtflags ());
      }
    };
  }
}

#endif
