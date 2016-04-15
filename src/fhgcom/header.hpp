#pragma once

#include <fhgcom/peer_info.hpp>

#include <util-generic/hash/std/tuple.hpp>

#include <array>
#include <string>
#include <tuple>

namespace fhg
{
  namespace com
  {
    namespace p2p
    {
      struct address_t
      {
        address_t() = default;
        address_t (std::string const& host, unsigned short port)
          : _port (port)
        {
          strncpy (_host.data(), host.c_str(), sizeof (_host) - 1);
          _host[sizeof (_host) - 1] = '\0';
        }
        address_t (host_t const& host, port_t const& port)
          : address_t (std::string (host), std::stoi (std::string (port)))
        {}

        //! \todo use std::string when no longer used in packets
        std::array<char, HOST_NAME_MAX + 1> _host;
        unsigned short _port;
      };

      inline std::string to_string (address_t const& address)
      {
        return std::string (address._host.data()) + ":"
          + std::to_string (address._port);
      }

      inline bool operator== (address_t const& lhs, address_t const& rhs)
      {
        return std::tie (lhs._host, lhs._port)
          == std::tie (rhs._host, rhs._port);
      }
      inline bool operator< (address_t const& lhs, address_t const& rhs)
      {
        return std::tie (lhs._host, lhs._port)
          <= std::tie (rhs._host, rhs._port);
      }
      inline bool operator!= (address_t const& lhs, address_t const& rhs)
      {
        return ! (lhs == rhs);
      }
    }
  }
}

namespace std
{
  template<> struct hash<fhg::com::p2p::address_t>
  {
    size_t operator() (fhg::com::p2p::address_t const& address) const
    {
      return hash<std::tuple<std::string, unsigned short>>()
        ( std::tuple<std::string, unsigned short>
            (address._host.data(), address._port)
        );
    }
  };
}

namespace fhg
{
  namespace com
  {
    namespace p2p
    {
      inline std::size_t hash_value (address_t const& address)
      {
        return std::hash<address_t>() (address);
      }

        static const int HELLO_PACKET = 0x8;

      struct header_t
      {
        header_t ()
          : type_of_msg (0)
          , length (0)
          , src()
          , dst()
        {}

        uint32_t type_of_msg;
        uint32_t length;           // size of payload in bytes
        address_t src;             // unique source address
        address_t dst;             // unique destination address
      } __attribute__ ((packed));
    }
  }
}
