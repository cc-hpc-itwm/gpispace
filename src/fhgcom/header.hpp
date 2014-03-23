#ifndef FHG_COM_HEADER_HPP
#define FHG_COM_HEADER_HPP 1

#include <cstring> // memset
#include <inttypes.h>
#include <iostream>
#include <string>

namespace fhg
{
  namespace com
  {
    namespace p2p
    {
      struct address_t
      {
        explicit
        address_t ();

        address_t (std::string const & name);

        uint8_t data[16];
      } __attribute__((packed));

      // standard operators
      bool operator==(address_t const& lhs, address_t const& rhs);
      bool operator!=(address_t const& lhs, address_t const& rhs);

      std::size_t hash_value(address_t const& u);

      std::string to_string (address_t const & a);

        static const int HELLO_PACKET = 0x8;

      struct header_t
      {
        header_t ()
          : type_of_msg (0)
          , length (0)
        {
          memset (&src, 0, sizeof(address_t));
          memset (&dst, 0, sizeof(address_t));
        }

        uint32_t type_of_msg;
        uint32_t length;           // size of payload in bytes
        address_t src;             // unique source address
        address_t dst;             // unique destination address
      } __attribute__ ((packed));
    }
  }
}

#endif
