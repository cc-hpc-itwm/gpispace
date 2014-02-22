#ifndef FHG_COM_TO_HEX_HPP
#define FHG_COM_TO_HEX_HPP 1

#include <string>
#include <sstream>
#include <ostream>
#include <iomanip>
#include <ios>

namespace fhg
{
  namespace com
  {
    namespace util
    {
      template <std::size_t MAX>
      struct basic_hex_converter
      {
        template <typename Iterator>
        static void convert (std::ostream & s, Iterator start, Iterator end)
        {
          std::ios_base::fmtflags flags(s.flags());
          s << std::hex;
          std::size_t cnt (0);
          while (start != end && cnt <= MAX)
          {
            s << "\\x"
              << std::setfill ('0')
              << std::setw (2)
              << (int)(*start++ & 0xff)
              ;
            ++cnt;
          }

          if (start != end)
          {
            s << " ...";
          }
          s.flags (flags);
        }

        template <typename Iterator>
        static std::string convert (Iterator start, Iterator end)
        {
          std::stringstream s;
          convert (s, start, end);
          return s.str();
        }

        template <typename Container>
        static std::string convert (Container c)
        {
          std::stringstream s;
          convert (s, c.begin(), c.end());
          return s.str();
        }
      };

      template <>
      struct basic_hex_converter<(std::size_t)(0)>
      {
        template <typename Iterator>
        static void convert (std::ostream & s, Iterator start, Iterator end)
        {
          std::ios_base::fmtflags flags(s.flags());
          s << std::hex;
          while (start != end)
          {
            s << "\\x"
              << std::setfill ('0')
              << std::setw (2)
              << (int)(*start++ & 0xff)
              ;
          }
          s.flags (flags);
        }

        template <typename Iterator>
        static std::string convert (Iterator start, Iterator end)
        {
          std::stringstream s;
          convert (s, start, end);
          return s.str();
        }

        template <typename Container>
        static std::string convert (Container c)
        {
          std::stringstream s;
          convert (s, c.begin(), c.end());
          return s.str();
        }
      };

      template <std::size_t MAX=0>
      struct basic_to_hex
      {
        basic_to_hex (std::string const & s)
          : s_(s)
        {}

        std::ostream & operator << (std::ostream & os) const
        {
          basic_hex_converter<MAX>::convert (os, s_.begin(), s_.end());
          return os;
        }
        std::string const & s_;
      };

      typedef basic_to_hex<64> log_raw;

      template <std::size_t T>
      inline std::ostream & operator << (std::ostream & os, const basic_to_hex<T> & h)
      {
        return h.operator<<(os);
      }
    }
  }
}

#endif
