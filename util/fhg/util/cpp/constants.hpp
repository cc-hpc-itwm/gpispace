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
        CONSTANT (path_type, pnetc   , "pnetc")
        CONSTANT (path_type, type    , pnetc() / "type")
        CONSTANT (path_type, op      , pnetc() / "op")
        CONSTANT (path_type, install , "$(LIB_DESTDIR)")
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

        inline std::string mod_so_name (const std::string& mod)
        {
          return extension::extend ("lib" + mod, extension::so());
        }

        inline std::string mod_so (const std::string & mod)
        {
          return (path::op() / mod_so_name (mod)).string();
        }

        inline std::string mod_so_install (const std::string& mod)
        {
          return (path::install() / mod_so_name (mod)).string();
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

        inline std::string dep (const std::string & mod)
        {
          const path_type path (path::op() / mod);

          return extension::extend (path.string(), extension::d());
        }

        inline std::string dep ( const std::string & mod
                               , const std::string & fun
                               )
        {
          const path_type path (path::op() / mod / fun);

          return extension::extend (path.string(), extension::d());
        }

        inline std::string stem (const std::string & mod)
        {
          return (path::op() / mod).string();
        }

        inline std::string stem ( const std::string & mod
                                , const std::string & fun
                                )
        {
          return (path::op() / mod / fun).string();
        }
      }

#undef CONSTANT
    }
  }
}

#endif
