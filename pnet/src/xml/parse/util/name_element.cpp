// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/name_element.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/util/expect.hpp>
#include <xml/parse/util/skip.hpp>

namespace xml
{
  namespace parse
  {
    std::string name_element ( xml_node_type*& node
                             , const boost::filesystem::path& path
                             )
    {
      skip (node, rapidxml::node_comment);

      try
        {
          expect (node, rapidxml::node_element, path);
        }
      catch (const error::missing_node &)
        {
          return "<missing_node>";
        }

      return std::string (node->name(), node->name_size());
    }
  }
}
