// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_ID_HPP
#define FUSE_ID_HPP 1

#include <string>

#include <boost/optional.hpp>

#include <iostream>
#include <iomanip>

#include <string>
#include <sstream>

#include <gpifs/util.hpp>
#include <cctype>
#include <inttypes.h>

namespace gpifs
{
  namespace id
  {
    typedef uint64_t id_t;

    static inline void toHex (std::ostream & os, const id_t & id)
    {
      const std::ios_base::fmtflags saved_flags (os.flags());
      const char saved_fill (os.fill (' '));
      const std::size_t saved_width (os.width (0));

      os << "0x"
	 << std::setw (sizeof (id) * 2)
	 << std::setfill ('0')
	 << std::hex
	 << id
	;

      os.flags (saved_flags);
      os.fill (saved_fill);
      os.width (saved_width);
    }

    static inline std::string toHex (const id_t & id)
    {
      std::ostringstream str;

      toHex (str, id);

      return str.str();
    }

    namespace detail
    {
      struct dec
      {
        static inline bool isdig (const char & c) { return isdigit (c); }
        static inline long val (const char & c) { return c - '0'; }
        static inline long base () { return 10; }
      };

      struct hex
      {
        static inline bool isdig (const char & c)
        {
          switch (c)
            {
            case 'a'...'f':
            case 'A'...'F': return true;
            default: return isdigit (c);
            }
        }

        static inline long val (const char & c)
        {
          switch (c)
            {
            case 'a'...'f': return 10 + (c - 'a');
            case 'A'...'F': return 10 + (c - 'A');
            default: return c - '0';
            }
        }

        static inline long base () { return 16; }
      };

      template<typename Base, typename IT>
      static inline void
      generic_parse (util::parse::parser<IT> & parser, id_t & id)
      {
        while (!parser.end() && Base::isdig (*parser))
          {
            id *= Base::base();
            id += Base::val (*parser);
            ++parser;
          }
      }
    }

    // ********************************************************************* //

    template<typename IT>
    static inline boost::optional<id_t>
    parse (util::parse::parser<IT> & parser)
    {
      util::parse::skip_space (parser);

      if (parser.end() || !isdigit (*parser))
        {
          parser.error_set ("expected digit");

          return boost::none;
        }
      else
        {
          id_t id (0);
          bool zero (false);

          while (!parser.end() && *parser == '0')
            {
              ++parser; zero = true;
            }

          if (!parser.end())
            {
              if (zero && tolower (*parser) == 'x')
                {
                  ++parser; detail::generic_parse<detail::hex> (parser, id);
                }
              else
                {
                  detail::generic_parse<detail::dec> (parser, id);
                }
            }

          return id;
        }
    }
  } // namespace id
} // namespace gpifs

#endif
