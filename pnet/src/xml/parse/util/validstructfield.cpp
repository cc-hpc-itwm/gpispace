// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/validstructfield.hpp>

namespace xml
{
  namespace parse
  {
    std::string validate_field_name ( const std::string& name
                                    , const boost::filesystem::path& path
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
