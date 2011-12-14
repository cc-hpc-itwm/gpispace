// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_CPP_CONSTANTS
#define _FHG_UTIL_CPP_CONSTANTS 1

#include <fhg/util/cpp/types.hpp>

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
        CONSTANT (path_type, pnetc, "pnetc")
        CONSTANT (path_type, type , pnetc() / "type")
        CONSTANT (path_type, op   , pnetc() / "op")
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

        inline std::string extend (const std::string & x, const std::string & e)
        {
          return x + "." + e;
        }
      }

      namespace make
      {
        inline std::string hpp (const std::string & name)
        {
          return extension::extend (name, extension::hpp());
        }

        inline std::string cpp (const std::string & name)
        {
          return extension::extend (name, extension::cpp());
        }

        inline std::string cpp ( const std::string & mod
                               , const std::string & fun
                               )
        {
          const path_type path (path::op() / mod / fun);

          return extension::extend (path.string(), extension::cpp());
        }

        inline std::string tmpl (const std::string & name)
        {
          return extension::extend (name, extension::tmpl());
        }

        inline std::string mod_so (const std::string & mod)
        {
          const path_type path (path::op() / ("lib" + mod));

          return extension::extend (path.string(), extension::so());
        }

        inline std::string obj (const std::string & mod)
        {
          const path_type path (path::op() / mod);

          return extension::extend (path.string(), extension::o());
        }

        inline std::string obj ( const std::string & mod
                               , const std::string & fun
                               )
        {
          const path_type path (path::op() / mod / fun);

          return extension::extend (path.string(), extension::o());
        }
      }

#undef CONSTANT
    }
  }
}

#endif
