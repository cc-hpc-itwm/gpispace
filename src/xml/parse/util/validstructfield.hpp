// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_VALID_STRUCT_FIELD
#define _XML_PARSE_VALID_STRUCT_FIELD

#include <boost/filesystem/path.hpp>

namespace xml
{
  namespace parse
  {
    std::string validate_field_name ( const std::string&
                                    , const boost::filesystem::path&
                                    );
  }
}

#endif
