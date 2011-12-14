#ifndef FHG_COM_MESSAGE_IO_HPP
#define FHG_COM_MESSAGE_IO_HPP 1

#include <ios>
#include <iostream>

#include <fhgcom/message.hpp>
#include <fhgcom/util/to_hex.hpp>

namespace fhg
{
  namespace com
  {
    inline
    std::ostream & operator << (std::ostream & os, p2p::header_t const & hdr)
    {
      std::ios_base::fmtflags flags(os.flags());
      os << std::hex;
      os << "FROM " << hdr.src
         << " TO " << hdr.dst
         << " V" << std::dec << hdr.version
         << " FLAGS 0x" << std::hex << hdr.flags
         << " TOM 0x" << std::hex << hdr.type_of_msg
         << " SIZE 0x" << std::hex << hdr.length;
        ;
      os.flags(flags);
      return os;
    }

    inline
    std::ostream & operator << (std::ostream & os, message_t const & m)
    {
      os << m.header;
      if (m.header.length)
      {
        os << " DATA "
           << util::basic_hex_converter<64>::convert(m.data);
      }
      return os;
    }
  }
}

#endif
