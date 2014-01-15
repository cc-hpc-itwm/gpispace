// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_DUMP_HPP
#define PNET_SRC_WE_TYPE_VALUE_DUMP_HPP

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

#endif
