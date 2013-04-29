// mirko.rahn@itwm.fraunhofer.de

#include <we/exception.hpp>

#include <we/type/signature/show.hpp>
#include <we/type/value/path/join.hpp>
#include <we/type/value/show.hpp>

namespace pnet
{
  namespace exception
  {
    type_error::type_error (const std::string& msg)
      : std::runtime_error ((boost::format ("type error: %1%") % msg).str())
    {}
    type_error::type_error (const boost::format& f)
      : std::runtime_error ((boost::format ("type error: %1%") % f).str())
    {}
    type_mismatch::type_mismatch
    ( const type::signature::signature_type& signature
    , const type::value::value_type& value
    , const std::list<std::string>& path
    )
      : type_error
        ( ( boost::format ( "type mismatch for field '%2%': expected type '%1%'"
                            ", value '%4%' has type '%3%'"
                          )
          % type::signature::show (signature)
          % type::value::path::join (path)
          % "<<signature (value)>>"
          % type::value::show (value)
          ).str()
        )
      , _signature (signature)
      , _value (value)
      , _path (path)
    {}
    missing_field::missing_field
    ( const type::signature::signature_type& signature
    , const std::list<std::string>& path
    )
      : type_error
        ( ( boost::format ("missing field '%2%' of type '%1%'")
          % type::signature::show (signature)
          % type::value::path::join (path)
          ).str()
        )
      , _signature (signature)
      , _path (path)
    {}
    unknown_field::unknown_field
    ( const type::value::value_type& value
    , const std::list<std::string>& path
    )
      : type_error
        ( ( boost::format ("unknown field '%1%' with value '%2%' of type '%3%'")
          % type::value::path::join (path)
          % type::value::show (value)
          % "<<signature (value)>>"
          ).str()
        )
      , _value (value)
      , _path (path)
    {}
  }
}
