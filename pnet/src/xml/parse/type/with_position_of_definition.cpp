// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/type/with_position_of_definition.hpp>

namespace xml
{
  namespace parse
  {
    with_position_of_definition::with_position_of_definition
      (const util::position_type& position_of_definition)
        : _position_of_definition (position_of_definition)
    {}

    const util::position_type&
    with_position_of_definition::position_of_definition() const
    {
      return _position_of_definition;
    }
  }
}
