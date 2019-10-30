// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <we/type/id.hpp>

#include <string>

namespace we
{
  namespace edge
  {
    //! \todo eliminate this, instead use subclasses of connection
    enum type {PT,PT_READ,TP,TP_MANY};

    bool is_pt_read (const type&);
    bool is_PT (const type&);

    std::string enum_to_string (const type&);
  }
}
