// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_SHOW_HPP
#define _FHG_UTIL_SHOW_HPP

#include <string>
#include <sstream>
#include <ios>
#include <stdexcept>
#include <typeinfo>

namespace fhg
{
  namespace util
  {
    template<typename T>
    inline
    std::string show ( const T & x
                     , std::ios_base::fmtflags flags=std::ios_base::fmtflags ()
                     )
    {
      std::ostringstream s;
      s.setf (flags);
      s << x;
      if (s.fail ())
      {
        throw std::runtime_error
          (std::string ("fhg::util::show: could not show ") + typeid(T).name ());
      }
      return s.str();
    }

    template<>
    inline std::string show<std::string> ( const std::string & x
                                         , std::ios_base::fmtflags flags
                                         )
    {
      return x;
    }

    template <typename T1, typename T2>
    inline std::string show ( std::pair<T1, T2> const & p
                            , const std::string & delimiter = ", "
                            , const std::string & start_end = "()"
                            )
    {
      std::ostringstream s;

      if (start_end.size() > 0)
        {
          s << start_end[0];
        }

      s << show (p.first);
      s << delimiter;
      s << show (p.second);

      if (start_end.size() > 1)
        {
          s << start_end[1];
        }
      else if (start_end.size() > 0)
        {
          s << start_end[0];
        }

      return s.str();
    }

    template <typename InputIterator>
    inline std::string show ( InputIterator first
                            , InputIterator last
                            , const std::string & delimiter = ", "
                            , const std::string & start_end = "[]"
                            )
    {
      InputIterator start (first);
      std::ostringstream s;

      if (start_end.size() > 0)
        {
          s << start_end[0];
        }

      while (first != last)
        {
          if (first != start)
            {
              s << delimiter;
            }
          s << show(*first);
          ++first;
        }

      if (start_end.size() > 1)
        {
          s << start_end[1];
        }
      else if (start_end.size() > 0)
        {
          s << start_end[0];
        }

      return s.str();
    }
  }
}

#endif
