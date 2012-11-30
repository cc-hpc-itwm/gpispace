// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_BOOST_HPP
#define FHG_UTIL_BOOST_HPP

#include <fhg/util/show.hpp>

#include <boost/optional.hpp>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      using namespace ::boost;

      template<typename T>
        std::ostream& operator<< ( std::ostream& s
                                 , const boost::optional<T>& o
                                 )
      {
          return s << (o ? ("Just " + fhg::util::show (*o)) : "Nothing");
      }

      template<typename T, typename U>
        boost::optional<U> fmap ( U (*f)(const T &)
                                , const boost::optional<T>& m
                                )
      {
        if (m)
        {
          return f (*m);
        }
        else
        {
          return boost::none;
        }
      }

      template<typename T>
        void fmap (void (*f)(const T &), const boost::optional<T> & m)
      {
        if (m)
        {
          f (*m);
        }
      }
    }
  }
}

#endif
