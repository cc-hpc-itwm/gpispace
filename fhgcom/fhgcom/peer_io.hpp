#ifndef FHG_COM_PEER_IO_HPP
#define FHG_COM_PEER_IO_HPP 1

#include <iostream>

#include <fhgcom/peer.hpp>

namespace fhg
{
  namespace com
  {
    inline
    std::ostream & operator << (std::ostream & os, peer_t const & p)
    {
      return os << p.name() << "@"
                << "[" << p.host() << "]:" << p.port();
    }
  }
}

#endif
