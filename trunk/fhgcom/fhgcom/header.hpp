#ifndef FHG_COM_HEADER_HPP
#define FHG_COM_HEADER_HPP 1

#include <inttypes.h>
#include <string>

namespace fhg
{
  namespace com
  {
    namespace p2p
    {
      typedef char address_t[16];

      struct header_t
      {
        int16_t version :  4; // set to fixed 1 currently
        int16_t flags   : 12; // set to fixed 0 currently
        int16_t _reserved;    // reserved bits for later use
        uint32_t payload;    // size of payload in bytes
        address_t src;         // unique source address
        address_t dst;         // unique destination address
      } __attribute__ ((packed));

      void translate_name (std::string const & name, address_t & addr);
    }
  }
}

#endif
