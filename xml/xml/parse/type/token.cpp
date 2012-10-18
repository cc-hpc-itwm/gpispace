// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/token.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      token_type::token_type ( const id::token& id
                             , const id::place& parent
                             , const signature::structured_t& structure
                             )
        : signature::desc_t (structure)
        , _id (id)
        , _parent (parent)
      { }

      token_type::token_type ( const id::token& id
                             , const id::place& parent
                             , const std::string& value
                             )
        : signature::desc_t (value)
        , _id (id)
        , _parent (parent)
      { }

      const id::token& token_type::id() const
      {
        return _id;
      }
      const id::place& token_type::parent() const
      {
        return _parent;
      }
    }
  }
}
