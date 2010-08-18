// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_VALID_NAME_HPP
#define _XML_PARSE_VALID_NAME_HPP 1

#include <xml/parse/error.hpp>

#include <boost/filesystem/path.hpp>

namespace xml
{
  namespace parse
  {
    inline std::string validate_name ( const std::string & name
                                     , const std::string & type
                                     , const boost::filesystem::path & path
                                     )
    {
      if (name.find ('.') != std::string::npos)
        {
          throw error::invalid_name_dot (name, type, path);
        }

      return name;
    }
  }
}

#endif
