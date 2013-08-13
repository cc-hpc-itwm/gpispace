// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_CPP_CONSTANTS
#define _FHG_UTIL_CPP_CONSTANTS 1

#include <boost/filesystem.hpp>

#include <string>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
#define CONSTANT(type,name,cons)                 \
        inline type const & name (void)          \
        {                                        \
          static type ret (cons);                \
                                                 \
          return ret;                            \
        }

      namespace access
      {
        inline std::string make (const std::string & x, const std::string & y)
        {
          return x + "::" + y;
        }

        inline std::string make ( const std::string & a
                                , const std::string & b
                                , const std::string & c
                                )
        {
          return make (a, make (b, c));
        }

        inline std::string make ( const std::string & a
                                , const std::string & b
                                , const std::string & c
                                , const std::string & d
                                )
        {
          return make (a, b, make (c, d));
        }

        inline std::string make ( const std::string & a
                                , const std::string & b
                                , const std::string & c
                                , const std::string & d
                                , const std::string & e
                                )
        {
          return make (a, b, c, make (d, e));
        }

        CONSTANT (std::string, type, "::pnetc::type")
        CONSTANT (std::string, value_type, "::value::type")
      }

#undef CONSTANT
    }
  }
}

#endif
