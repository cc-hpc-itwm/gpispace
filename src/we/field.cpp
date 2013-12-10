// mirko.rahn@itwm.fraunhofer.de

#include <we/field.hpp>
#include <we/type/value/peek.hpp>

namespace pnet
{
  const type::value::value_type& field
    ( const std::string& f
    , const type::value::value_type& v
    , const type::signature::signature_type& signature
    )
  {
    boost::optional<const type::value::value_type&> field
      (type::value::peek (f, v));

    if (!field)
    {
      throw exception::missing_field ( signature
                                     , v
                                     , std::list<std::string> (1, f)
                                     );
    }

    return *field;
  }
}
