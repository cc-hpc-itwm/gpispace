// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_VALID_STRUCT_FIELD
#define _XML_PARSE_VALID_STRUCT_FIELD

#include <xml/parse/error.hpp>

#include <boost/filesystem/path.hpp>

#include <we/type/literal/cpp.hpp>

#include <xml/parse/util/valid_name.hpp>

namespace xml
{
  namespace parse
  {
    inline std::string
    validate_field_name ( const std::string & name
                        , const boost::filesystem::path & path
                        )
    {
      if (literal::cpp::reserved (name))
        {
          throw error::invalid_field_name (name, path);
        }

      return validate_name (name, "fieldname", path);
    }
  }
}

#endif
