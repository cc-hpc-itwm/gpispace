// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_ID_HPP
#define FUSE_ID_HPP 1

#include <string>

#include <boost/optional.hpp>

#include <util.hpp>

namespace gpifs
{
  namespace id
  {
    typedef unsigned long id_t;

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
            case 'a'...'e':
            case 'A'...'E': return true;
            default: return isdigit (c);
            }
        }

        static inline long val (const char & c)
        {
          switch (c)
            {
            case 'a'...'e': return 10 + (c - 'a');
            case 'A'...'E': return 10 + (c - 'A');
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

          return boost::optional<id_t> (boost::none);
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

          return boost::optional<id_t> (id);
        }
    }
  } // namespace id
} // namespace gpifs

#endif
