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
      expect_none_or (node, rapidxml::node_element, path);

      if (!node)
      {
        return "<missing_node>";
      }

      return std::string (node->name(), node->name_size());
    }
  }
}
