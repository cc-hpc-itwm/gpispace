// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/cctype.hpp>
#include <fhg/util/num.hpp>
#include <fhg/util/parse/error.hpp>

#include <util-generic/fallthrough.hpp>

#include <limits>

namespace fhg
{
  namespace util
  {
    namespace
    {
      template<typename I>
        struct hex
      {
        static inline bool isdig (char const& c)
        {
          switch (c)
          {
          case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
          case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': return true;
          default: return fhg::util::isdigit (c);
          }
        }
        static inline I val (char const& c)
        {
          switch (c)
          {
          case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': return 10 + (c - 'a');
          case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': return 10 + (c - 'A');
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
        static inline bool isdig (char const& c)
        {
          return fhg::util::isdigit (c);
        }
        static inline I val (char const& c)
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
                                            , bool has_sign
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
                                       , bool has_sign = false
                                       )
      {
        if (pos.end() || !fhg::util::isdigit(*pos))
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

      inline bool read_sign (parse::position& pos)
      {
        if (!pos.end())
        {
          if (*pos == '-')
          {
            ++pos;

            return true;
          }
          else if (*pos == '+')
          {
            ++pos;

            return false;
          }
        }

        return false;
      }

      double read_fraction (unsigned long ipart, parse::position& pos)
      {
        double x (ipart);

        double frac (0.0);
        double fak (0.1);

        while (!pos.end() && fhg::util::isdigit (*pos))
        {
          frac += double(*pos - '0') * fak;
          fak *= 0.1;
          ++pos;
        }

        x = (x < 0) ? (x - frac) : (x + frac);

        if (!pos.end() && (*pos == 'e' || *pos == 'E'))
        {
          ++pos;

          const bool negative (read_sign (pos));
          const unsigned long ex (read_ulong (pos));

          if (negative)
          {
            for (unsigned long i (0); i < ex; ++i)
            {
              x /= 10;
            }
          }
          else
          {
            for (unsigned long i (0); i < ex; ++i)
            {
              x *= 10;
            }
          }
        }

        return x;
      }
    }

    unsigned long read_ulong (parse::position& pos)
    {
      return generic_read_integral<unsigned long> (pos);
    }
    unsigned int read_uint (parse::position& pos)
    {
      return generic_read_integral<unsigned int> (pos);
    }

    namespace
    {
      template<typename From, typename To>
        To cast (From const& x, parse::position const& pos)
      {
        if (x > static_cast<From> (std::numeric_limits<To>::max()))
        {
          throw parse::error::value_too_big<From, To> (x, pos);
        }

        return static_cast<To> (x);
      }

      template<>
        float cast (unsigned long const& x, parse::position const&)
      {
        // Always fits. Check would fail though due to max float not
        // fitting unsigned long.
        return static_cast<float> (x);
      }
    }

#define SIGNED(e...) num_type (negative ? -(e) : (e))

    num_type read_num (parse::position& pos)
    {
      const bool negative (read_sign (pos));
      const unsigned long ul (read_ulong (pos));

      if (!pos.end())
      {
        switch (*pos)
        {
        case 'l':
        case 'L':
          ++pos;

          return SIGNED (cast<unsigned long, long> (ul, pos));

        case 'u':
        case 'U':
          ++pos;

          if (!pos.end() && (*pos == 'l' || *pos == 'L'))
          {
            ++pos;

            return SIGNED (ul);
          }
          else
          {
            return SIGNED (cast<unsigned long, unsigned int> (ul, pos));
          }

        case 'f':
        case 'F':
          ++pos;

          return SIGNED (cast<unsigned long, float> (ul, pos));

        default:
          if (!pos.end())
          {
            switch (*pos)
            {
            case '.':
              ++pos;
              FHG_UTIL_FALLTHROUGH;
            case 'e':
            case 'E':
              {
                auto const d (read_fraction (ul, pos));

                if (!pos.end() && (*pos == 'f' || *pos == 'F'))
                {
                  ++pos;

                  return SIGNED (cast<double, float> (d, pos));
                }

                return SIGNED (d);
              }
            default:
              break;
            }
          }
        }
      }

      return SIGNED (cast<unsigned long, int> (ul, pos));
    }

#undef SIGNED
  }
}
