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

      namespace path
      {
        CONSTANT (boost::filesystem::path, pnetc   , "pnetc")
        CONSTANT (boost::filesystem::path, type    , pnetc() / "type")
        CONSTANT (boost::filesystem::path, op      , pnetc() / "op")
        CONSTANT (boost::filesystem::path, install , "$(LIB_DESTDIR)")
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

        CONSTANT (std::string, type, make ("", "pnetc", "type"))
        CONSTANT (std::string, value_type, make ("", "value", "type"))
      }

      namespace extension
      {
        CONSTANT (std::string, hpp, "hpp")
        CONSTANT (std::string, cpp, "cpp")
        CONSTANT (std::string, tmpl, "cpp_tmpl")
        CONSTANT (std::string, so, "so")
        CONSTANT (std::string, o, "o")
        CONSTANT (std::string, d, "d")
      }

      namespace make
      {
        inline std::string mod_so_name (const std::string& mod)
        {
          return "lib" + mod + "." + extension::so();
        }

        inline std::string mod_so (const std::string & mod)
        {
          return (path::op() / mod_so_name (mod)).string();
        }

        inline std::string mod_so_install (const std::string& mod)
        {
          return (path::install() / mod_so_name (mod)).string();
        }
      }

#undef CONSTANT
    }
  }
}

#endif
