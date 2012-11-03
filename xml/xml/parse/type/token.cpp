// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/token.hpp>

#include <xml/parse/id/mapper.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      token_type::token_type ( ID_CONS_PARAM(token)
                             , PARENT_CONS_PARAM(place)
                             , const signature::structured_t& structure
                             )
        : signature::desc_t (structure)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
      {
        _id_mapper->put (_id, *this);
      }

      token_type::token_type ( ID_CONS_PARAM(token)
                             , PARENT_CONS_PARAM(place)
                             , const std::string& value
                             )
        : signature::desc_t (value)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
      {
        _id_mapper->put (_id, *this);
      }
    }
  }
}
