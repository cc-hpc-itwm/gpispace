#ifndef FHG_COM_HEADER_HPP
#define FHG_COM_HEADER_HPP 1

#include <inttypes.h>
#include <string>
#include <iostream>
#include <cstring> // memset

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
      bool operator<(address_t const& lhs, address_t const& rhs);
      bool operator>(address_t const& lhs, address_t const& rhs);
      bool operator<=(address_t const& lhs, address_t const& rhs);
      bool operator>=(address_t const& lhs, address_t const& rhs);

      void swap(address_t& lhs, address_t& rhs);

      std::size_t hash_value(address_t const& u);

      std::ostream & operator<<(std::ostream & os, address_t const &);
      std::string to_string (address_t const & a);

      struct header_t
      {
        header_t ()
          : version (0)
          , flags (0)
          , payload (0)
        {
          memset (&src, 0, sizeof(address_t));
          memset (&dst, 0, sizeof(address_t));
        }

        int16_t version :  4; // set to fixed 0 currently
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
