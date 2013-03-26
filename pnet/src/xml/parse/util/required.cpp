// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/required.hpp>

#include <xml/parse/error.hpp>

namespace xml
{
  namespace parse
  {
    std::string
    required ( const std::string& pre
             , const xml_node_type* node
             , const Ch* attr
             , const boost::filesystem::path& path
             )
    {
      if (!node->first_attribute (attr))
        {
          throw error::missing_attr (pre, attr, path);
        }

      return std::string ( node->first_attribute (attr)->value()
                         , node->first_attribute (attr)->value_size()
                         );
    }
  }
}
