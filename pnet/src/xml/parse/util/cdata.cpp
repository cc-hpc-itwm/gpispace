// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/cdata.hpp>

#include <xml/parse/error.hpp>

#include <xml/parse/util/expect.hpp>
#include <xml/parse/util/skip.hpp>

namespace xml
{
  namespace parse
  {
    void cdata (xml_node_type*& node, const boost::filesystem::path& path)
    {
      skip (node, rapidxml::node_comment);

      try
        {
          expect (node, rapidxml::node_data, rapidxml::node_cdata, path);
        }
      catch (const error::missing_node&)
        {
          // do nothing, there was just no (c)data given
        }

      return;
    }
  }
}
