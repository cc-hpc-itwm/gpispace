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
                             , const id::place& parent
                             , const signature::structured_t& structure
                             )
        : signature::desc_t (structure)
        , ID_INITIALIZE()
        , _parent (parent)
      {
        _id_mapper->put (_id, *this);
      }

      token_type::token_type ( ID_CONS_PARAM(token)
                             , const id::place& parent
                             , const std::string& value
                             )
        : signature::desc_t (value)
        , ID_INITIALIZE()
        , _parent (parent)
      {
        _id_mapper->put (_id, *this);
      }

      const id::place& token_type::parent() const
      {
        return _parent;
      }
    }
  }
}
