#include <iml/util/num.hpp>

#include <iml/util/parse/error.hpp>

#include <limits>

namespace fhg
{
  namespace iml
  {
    namespace util
    {
      namespace
      {
        template<typename I>
          struct hex
        {
          static inline bool isdig (const char& c)
          {
            switch (c)
            {
            case 'a'...'f':
            case 'A'...'F': return true;
            default: return isdigit (c);
            }
          }
          static inline I val (const char& c)
          {
            switch (c)
            {
            case 'a'...'f': return I (10 + (c - 'a'));
            case 'A'...'F': return I (10 + (c - 'A'));
            default: return I (c - '0');
            }
          }
          static inline I base()
          {
            return 16;
          }
        };

        template<typename I>
          struct dec
        {
          static inline bool isdig (const char & c)
          {
            return isdigit (c);
          }
          static inline I val (const char & c)
          {
            return I (c - '0');
          }
          static const I base()
          {
            return 10;
          }
        };

        template<typename Base, typename I>
          inline I generic_base_read_integral ( parse::position& pos
                                              , const bool has_sign
                                              )
        {
          I l (0);

          if (has_sign)
          {
            while (!pos.end() && Base::isdig (*pos))
            {
              if (l < std::numeric_limits<I>::min() / Base::base())
              {
                throw parse::error::unexpected_digit<I> (pos);
              }

              l *= Base::base();

              if (l < std::numeric_limits<I>::min() + Base::val (*pos))
              {
                throw parse::error::unexpected_digit<I> (pos);
              }

              l -= Base::val (*pos);

              ++pos;
            }
          }
          else
          {
            while (!pos.end() && Base::isdig (*pos))
            {
              if (l > std::numeric_limits<I>::max() / Base::base())
              {
                throw parse::error::unexpected_digit<I> (pos);
              }

              l *= Base::base();

              if (l > std::numeric_limits<I>::max() - Base::val (*pos))
              {
                throw parse::error::unexpected_digit<I> (pos);
              }

              l += Base::val (*pos);

              ++pos;
            }
          }

          return l;
        }

        template<typename I>
          inline I generic_read_integral ( parse::position& pos
                                         , const bool has_sign = false
                                         )
        {
          if (pos.end() || !isdigit(*pos))
          {
            throw parse::error::expected ("digit", pos);
          }

          bool leading_zero (false);

          while (!pos.end() && *pos == '0')
          {
            ++pos;

            leading_zero = true;
          }

          if (!pos.end())
          {
            if (leading_zero && (*pos == 'x' || *pos == 'X'))
            {
              ++pos;

              return generic_base_read_integral<hex<I>, I> (pos, has_sign);
            }
            else
            {
              return generic_base_read_integral<dec<I>,I> (pos, has_sign);
            }
          }

          return 0;
        }
      }

      unsigned int read_uint (parse::position& pos)
      {
        return generic_read_integral<unsigned int> (pos);
      }
    }
  }
}
