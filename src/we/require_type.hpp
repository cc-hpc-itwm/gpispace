#pragma once

#include <we/type/value.hpp>
#include <we/type/signature.hpp>

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
}
