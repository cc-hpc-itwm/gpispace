// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/get.hpp>

#include <we/type/value/show.hpp>
#include <we/type/literal/show.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/split.hpp>

namespace value
{
  namespace visitor
  {
    namespace exception
    {
      missing_field::missing_field (const std::string& name)
        : std::runtime_error ("get_field: missing field " + name)
      {}


      cannot_get_field_from_literal::cannot_get_field_from_literal
        ( const std::string& name
        , const literal::type& l
        )
          : std::runtime_error
            ( ( boost::format ("cannot get field %1% from the literal %2%")
              % name % literal::show (l)
              ).str()
            )
      {}

      empty_path::empty_path()
        : std::runtime_error ("get: empty path")
      {}
    }
  }
}
