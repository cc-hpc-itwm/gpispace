// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_VALID_PREFIX
#define _XML_PARSE_VALID_PREFIX

#include <boost/filesystem/path.hpp>

namespace xml
{
  namespace parse
  {
    std::string validate_prefix ( const std::string& name
                                , const std::string& type
                                , const boost::filesystem::path& path
                                );
  }
}

#endif
