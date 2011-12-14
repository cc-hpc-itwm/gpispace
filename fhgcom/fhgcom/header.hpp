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

      struct type_of_message_traits
      {
        // user data 0 -> 7
        static const int USER_DATA_PACKET = 0x0;

        // system data 8 -> 15
        static const int SYSTEM_PACKET_FLAG = 0x8;
        static const int HELLO_PACKET = 0x8;

        inline static bool is_user_data (const int tom)
        {
          return (tom & SYSTEM_PACKET_FLAG) == 0;
        }
        inline static bool is_system_data (const int tom)
        {
          return (tom & SYSTEM_PACKET_FLAG) != 0;
        }
      };

      struct header_t
      {
        header_t ()
          : version (0)
          , flags (0)
          , type_of_msg (0)
          , length (0)
        {
          memset (&src, 0, sizeof(address_t));
          memset (&dst, 0, sizeof(address_t));
        }

        uint32_t version     :  4; // set to fixed 0 currently
        uint32_t flags       : 12; // set to fixed 0 currently
        uint32_t type_of_msg :  4; // see type_of_message_traits
        uint32_t reserved    : 12; // reserved bits for later use
        uint32_t length;           // size of payload in bytes
        address_t src;             // unique source address
        address_t dst;             // unique destination address
      } __attribute__ ((packed));
    }
  }
}

#endif
