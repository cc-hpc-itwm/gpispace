// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_FIELD_HPP
#define PNET_SRC_WE_FIELD_HPP

#include <we/exception.hpp>
#include <we/type/signature.hpp>
#include <we/type/value.hpp>

#include <list>
#include <string>

namespace pnet
{
  const type::value::value_type& field ( const std::string&
                                       , const type::value::value_type&
                                       , const type::signature::signature_type&
                                       );

  template<typename T>
    const T& field_as ( const std::string& f
                      , const type::value::value_type& v
                      , const type::signature::signature_type& signature
                      )
  {
    const type::value::value_type& value (field (f, v, signature));

    const T* x (boost::get<const T> (&value));

    if (!x)
    {
      throw exception::type_mismatch ( signature
                                     , value
                                     , std::list<std::string> (1, f)
                                     );
    }

    return *x;
  }
}

#endif
