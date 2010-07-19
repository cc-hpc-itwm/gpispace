// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_SPECIALIZE_HPP
#define _XML_PARSE_TYPE_SPECIALIZE_HPP

#include <xml/parse/types.hpp>

#include <iostream>

#include <boost/unordered_map.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct specialize_type
      {
        std::string name;
        std::string use;
        int level;
        type_map_type type_map_in;
        type_map_type type_map_out;
      };

      std::ostream & operator << (std::ostream & s, const specialize_type & sp)
      {
        s << level (sp.level) << "specialize (" << std::endl;
        s << level (sp.level + 1) << "name = " << sp.name << std::endl;
        s << level (sp.level + 1) << "use = " << sp.use << std::endl;

        s << level (sp.level + 1) << "type-map-in = " << std::endl;

        for ( type_map_type::const_iterator pos (sp.type_map_in.begin())
            ; pos != sp.type_map_in.end()
            ; ++pos
            )
          {
            s << level (sp.level + 2)
              << pos->first << " => " << pos->second
              << std::endl
              ;
          }

        s << level (sp.level + 1) << "type-map-out = " << std::endl;

        for ( type_map_type::const_iterator pos (sp.type_map_out.begin())
            ; pos != sp.type_map_out.end()
            ; ++pos
            )
          {
            s << level (sp.level + 2)
              << pos->first << " => " << pos->second
              << std::endl
              ;
          }

        return s << level (sp.level) << ") // specialize";
      }
    }
  }
}

#endif
