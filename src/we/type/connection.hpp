// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_CONNECTION_HPP
#define _WE_TYPE_CONNECTION_HPP

#include <we/type/id.hpp>

#include <string>

namespace petri_net
{
  namespace edge
  {
    //! \todo eliminate this, instead use subclasses of connection
    enum type {PT,PT_READ,TP};

    bool is_pt_read (const type&);
    bool is_PT (const type&);

    std::string enum_to_string (const type&);
  }
}

#endif
