// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_MOD_HPP
#define _XML_PARSE_TYPE_MOD_HPP

#include <string>
#include <iostream>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct mod
      {
      public:
        std::string name;
        std::string function;
        boost::filesystem::path path;

        mod ( const std::string & _name
            , const std::string & _function
            , const boost::filesystem::path & _path
            )
          : name (_name)
          , function (_function)
          , path (_path)
        {}
      };

      std::ostream & operator << (std::ostream & s, const mod & m)
      {
        return s << "mod ("
                 << "mod = " << m.name 
                 << ", function = " << m.function 
                 << ", path = " << m.path
                 << ")"
          ;
      }
    }
  }
}

#endif
