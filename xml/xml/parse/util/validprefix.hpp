// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_VALID_PREFIX
#define _XML_PARSE_VALID_PREFIX

#include <xml/parse/error.hpp>

#include <boost/filesystem/path.hpp>

#include <rewrite/validprefix.hpp>

namespace xml
{
  namespace parse
  {
    inline std::string validate_prefix ( const std::string & name
                                       , const std::string & type
                                       , const boost::filesystem::path & path
                                       )
    {
      if (rewrite::has_magic_prefix (name))
        {
          throw error::invalid_prefix (name, type, path);
        }

      return name;
    }
  }
}

#endif
