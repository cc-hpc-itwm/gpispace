// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_READ_HPP
#define _FHG_UTIL_READ_HPP 1

namespace fhg
{
  namespace util
  {
    template<typename T>
    inline T read (const std::string & showed)
    {
      T x; std::istringstream i(showed); i >> x; return x;
    }

    template<>
    inline std::string read (const std::string & s) { return s; }

    template <typename T>
    struct reader
    {
      inline static T read (std::string const & s)
      {
        return fhg::util::read<T>(s);
      }
    };
  }
}

#endif
