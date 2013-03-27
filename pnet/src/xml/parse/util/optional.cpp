// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/optional.hpp>

namespace xml
{
  namespace parse
  {
    boost::optional<std::string> optional ( const xml_node_type* node
                                          , const Ch* attr
                                          )
    {
      if (node->first_attribute (attr))
      {
        return std::string ( node->first_attribute (attr)->value()
                           , node->first_attribute (attr)->value_size()
                           );
      }
      return boost::none;
    }
  }
}
