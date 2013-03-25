// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_WITH_POSITION_OF_DEFINITION_HPP
#define _XML_PARSE_TYPE_WITH_POSITION_OF_DEFINITION_HPP

#include <xml/parse/util/position.hpp>

namespace xml
{
  namespace parse
  {
    class with_position_of_definition
    {
    public:
      with_position_of_definition (const util::position_type&);

      const util::position_type& position_of_definition() const;

    protected:
      util::position_type _position_of_definition;
    };
  }
}

#endif
