// mirko.rahn@itwm.fraunhofer.de

#include <we/exception.hpp>

#include <we/type/signature/show.hpp>
#include <we/type/value/path/join.hpp>
#include <we/type/value/show.hpp>
#include <we/signature_of.hpp>

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
          % type::signature::show (signature_of (value))
          % type::value::show (value)
          ).str()
        )
      , _signature (signature)
      , _value (value)
      , _path (path)
    {}
    missing_field::missing_field
    ( const type::signature::signature_type& signature
    , const type::value::value_type& value
    , const std::list<std::string>& path
    )
      : type_error
        ( ( boost::format ("missing field '%2%' of type '%1%' in value '%3%'")
          % type::signature::show (signature)
          % type::value::path::join (path)
          % type::value::show (value)
          ).str()
        )
      , _signature (signature)
      , _value (value)
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
          % type::signature::show (signature_of (value))
          ).str()
        )
      , _value (value)
      , _path (path)
    {}
    eval::eval ( const expr::token::type& token
               , const type::value::value_type& x
               )
      : type_error
        ( ( boost::format ("eval %1% (%2%)")
          % token
          % type::value::show (x)
          ).str()
        )
      , _token (token)
      , _values()
    {
      _values.push_back (x);
    }
    eval::eval ( const expr::token::type& token
               , const type::value::value_type& l
               , const type::value::value_type& r
               )
      : type_error
        ( ( boost::format ("eval %1% (%2%, %3%)")
          % expr::token::show (token)
          % type::value::show (l)
          % type::value::show (r)
          ).str()
        )
      , _token (token)
      , _values()
    {
      _values.push_back (l);
      _values.push_back (r);
    }
    missing_binding::missing_binding (const std::string& key)
      : std::runtime_error
        ((boost::format ("missing binding for: ${%1%}") % key).str())
      , _key (key)
    {}
    could_not_resolve::could_not_resolve ( const std::string& type
                                         , const std::list<std::string>& path
                                         )
      : std::runtime_error
        ((boost::format ("could not resolve type '%1%' for field '%2%'")
         % type::value::path::join (path)
         % type
         ).str()
        )
      , _type (type)
      , _path (path)
    {}
  }
}
