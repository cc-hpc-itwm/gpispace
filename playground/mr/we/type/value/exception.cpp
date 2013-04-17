// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/exception.hpp>

#include <we/type/value/path/join.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/signature/show.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace exception
      {
        type_error::type_error (const std::string& msg)
          : std::runtime_error ((boost::format ("type error: %1%") % msg).str())
        {}
        type_error::type_error (const boost::format& f)
          : std::runtime_error ((boost::format ("type error: %1%") % f).str())
        {}
        type_mismatch::type_mismatch ( const signature_type& signature
                                     , const value_type& value
                                     , const std::list<std::string>& path
                                     )
          : type_error
            ( ( boost::format ( "type mismatch for field %2%: expected %1%"
                                ", value has %3%"
                              )
              % as_signature (signature)
              % path::join (path)
              % as_signature (value)
              ).str()
            )
        {}
        missing_field::missing_field ( const signature_type& signature
                                     , const std::list<std::string>& path
                                     )
          : type_error
            ( ( boost::format ( "missing field %2% of type %1%")
              % as_signature (signature)
              % path::join (path)
              ).str()
            )
        {}
        unknown_field::unknown_field ( const value_type& value
                                     , const std::list<std::string>& path
                                     )
          : type_error
            ( ( boost::format ("unknown field %1% with value %2%")
              % path::join (path)
              % "value"
              ).str()
            )
        {}
      }
    }
  }
}
