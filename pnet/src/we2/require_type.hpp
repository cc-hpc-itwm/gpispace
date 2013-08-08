// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_REQUIRE_TYPE_HPP
#define PNET_SRC_WE_REQUIRE_TYPE_HPP

#include <we2/type/value.hpp>
#include <we2/type/signature.hpp>

namespace pnet
{
  const type::value::value_type& require_type
    ( const type::value::value_type&
    , const type::signature::signature_type&
    );
  const type::value::value_type& require_type
    ( const type::value::value_type&
    , const type::signature::signature_type&
    , const std::string& field
    );

  //! \note does not require the same order of fields in the value as in
  //! the signature
  const type::value::value_type& require_type_relaxed
    ( const type::value::value_type&
    , const type::signature::signature_type&
    , const std::string& field
    );
}

#endif
