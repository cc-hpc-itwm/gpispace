// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_ALPHANUM_HPP
#define FHG_UTIL_ALPHANUM_HPP

#include <cctype>
#include <cstdlib>
#include <functional>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace alphanum
    {
      namespace
      {
        /* Based on http://www.davekoelle.com/files/alphanum.hpp
           The Alphanum Algorithm is an improved sorting algorithm for strings
           containing numbers.  Instead of sorting numbers in ASCII order like a
           standard sort, this algorithm sorts numbers in numeric order.

           The Alphanum Algorithm is discussed at http://www.DaveKoelle.com

           This implementation is Copyright (c) 2008 Dirk Jagdmann <doj@cubic.org>.
           It is a cleanroom implementation of the algorithm and not derived by
           other's works. In contrast to the versions written by Dave Koelle this
           source code is distributed with the libpng/zlib license.

           This software is provided 'as-is', without any express or implied
           warranty. In no event will the authors be held liable for any damages
           arising from the use of this software.

           Permission is granted to anyone to use this software for any purpose,
           including commercial applications, and to alter it and redistribute it
           freely, subject to the following restrictions:

           1. The origin of this software must not be misrepresented; you
           must not claim that you wrote the original software. If you use
           this software in a product, an acknowledgment in the product
           documentation would be appreciated but is not required.

           2. Altered source versions must be plainly marked as such, and
           must not be misrepresented as being the original software.

           3. This notice may not be removed or altered from any source
           distribution. */
        /* $Header: /code/doj/alphanum.hpp,v 1.3 2008/01/28 23:06:47 doj Exp $ */

        /**
           compare l and r with strcmp() semantics, but using
           the "Alphanum Algorithm". This function is designed to read
           through the l and r strings only one time, for
           maximum performance. It does not allocate memory for
           substrings. It can either use the C-library functions isdigit()
           and atoi() to honour your locale settings, when recognizing
           digit characters when you "#define ALPHANUM_LOCALE=1" or use
           it's own digit character handling which only works with ASCII
           digit characters, but provides better performance.

           @param l NULL-terminated C-style string
           @param r NULL-terminated C-style string
           @return negative if l<r, 0 if l equals r, positive if l>r
        */
        int alphanum_impl (const char *l, const char *r)
        {
          enum mode_t { STRING, NUMBER } mode (STRING);

          while (*l && *r)
          {
            if (mode == STRING)
            {
              char l_char, r_char;
              while ((l_char = *l) && (r_char = *r))
              {
                const bool l_digit (isdigit (l_char));
                const bool r_digit (isdigit (r_char));

                if (l_digit && r_digit)
                {
                  mode = NUMBER;
                  break;
                }

                if (l_digit) return -1;
                if (r_digit) return +1;

                const int diff (l_char - r_char);
                if(diff != 0) return diff;

                ++l;
                ++r;
              }
            }
            else
            {
              char *end;
              const unsigned long l_int (strtoul (l, &end, 10));
              l = end;

              const unsigned long r_int (strtoul (r, &end, 10));
              r = end;

              const long diff (l_int - r_int);
              if (diff != 0) return diff;

              mode = STRING;
            }
          }

          if (*r) return -1;
          if (*l) return +1;
          return 0;
        }
      }

      struct less
        : public std::binary_function<std::string, std::string, bool>
      {
        bool operator() (const std::string& left, const std::string& right) const
        {
          return alphanum_impl (left.c_str(), right.c_str()) < 0;
        }
      };

      struct less_equal
        : public std::binary_function<std::string, std::string, bool>
      {
        bool operator() (const std::string& left, const std::string& right) const
        {
          return alphanum_impl (left.c_str(), right.c_str()) <= 0;
        }
      };
    }
  }
}

#endif
