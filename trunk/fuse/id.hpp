// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_ID_HPP
#define FUSE_ID_HPP 1

#include <string>

namespace gpi_fuse
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
      static inline void generic_parse (IT & pos, const IT & end, id_t & id)
      {
        while (pos != end && Base::isdig (*pos))
          {
            id *= Base::base();
            id += Base::val (*pos);
            ++pos;
          }
      }
    }

    template<typename IT>
    static inline id_t parse (IT & pos, const IT & end)
    {
      id_t id (0);

      if (pos != end && isdigit (*pos))
        {
          bool zero (false);

          while (pos != end && *pos == '0')
            {
              ++pos; zero = true;
            }

          if (pos != end)
            {
              if (zero && (*pos == 'x' || *pos == 'X'))
                {
                  ++pos; detail::generic_parse<detail::hex> (pos, end, id);
                }
              else
                {
                  detail::generic_parse<detail::dec> (pos, end, id);
                }
            }
        }

      return id;
    }
  } // namespace id
} // namespace gpi_fuse

#endif
