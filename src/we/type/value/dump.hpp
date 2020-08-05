#pragma once

#include <we/type/value.hpp>

#include <fhg/util/xml.fwd.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      fhg::util::xml::xmlstream& dump ( fhg::util::xml::xmlstream&
                                      , const value_type&
                                      );
    }
  }
}
