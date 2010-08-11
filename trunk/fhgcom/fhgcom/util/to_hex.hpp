#ifndef FHG_COM_TO_HEX_HPP
#define FHG_COM_TO_HEX_HPP 1

#include <string>
#include <ostream>
#include <iomanip>
#include <ios>

namespace fhg
{
  namespace com
  {
    namespace util
    {
      struct to_hex
      {
        to_hex (std::string const & s, const std::size_t max_dump = 128)
          : s_(s)
          , max_(max_dump)
        {}

        std::ostream & operator << (std::ostream & os) const
        {
          std::string::const_iterator c (s_.begin());
          std::ios_base::fmtflags flags(os.flags());
          os << std::hex;
          std::size_t cnt (0);
          while (c != s_.end() && ++cnt <= max_)
          {
            os << "\\x"
               << std::setfill ('0')
               << std::setw (2)
               << (int)(*c++ & 0xff)
              ;
          }
          if (c != s_.end())
          {
            os << " ...";
          }
          os.flags (flags);
          return os;
        }
        std::string const & s_;
        const std::size_t max_;
      };

      inline std::ostream & operator << (std::ostream & os, const to_hex & h)
      {
        return h.operator<<(os);
      }
    }
  }
}

#endif
