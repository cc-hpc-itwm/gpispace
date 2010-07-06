// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_STRUCT_HPP
#define _XML_PARSE_TYPE_STRUCT_HPP

#include <we/type/signature.hpp>

#include <iostream>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct struct_t
      {
        std::string name;
        signature::desc_t sig;
        boost::filesystem::path path;
        int level;
      };

      std::ostream & operator << (std::ostream & s, const struct_t & st)
      {
        return s << level(st.level) << "struct (" << std::endl
                 << level(st.level+1) << "name = " << st.name << std::endl
                 << level(st.level+1) << "sig = " << st.sig << std::endl
                 << level(st.level+1) << "path = " << st.path << std::endl
                 << level(st.level) << ") // struct";
      }
    }
  }
}

#endif
