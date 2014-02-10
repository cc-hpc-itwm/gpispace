// mirko.rahn@itwm.fhg.de

#ifndef FHG_UTIL_BOOST_TEST_PRINTER_CONTAINER_HPP
#define FHG_UTIL_BOOST_TEST_PRINTER_CONTAINER_HPP

#include <fhg/util/boost/test.hpp>
#include <fhg/util/first_then.hpp>

#include <boost/foreach.hpp>
#include <boost/function.hpp>

#include <string>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      using namespace ::boost;

      namespace test
      {
        namespace printer
        {
          template<typename T>
          class print_element_default
          {
          public:
            void operator() (std::ostream& os, T const& x) const
            {
              os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (x);
            }
          };

          template<typename Container>
          std::ostream& container
            ( std::ostream& os
            , Container const& c
            , std::string const& prefix
            , std::string const& suffix
            , boost::function <void ( std::ostream&
                                    , typename Container::value_type const&
                                    )
                              > const& print_element
              = print_element_default<typename Container::value_type>()
            )
          {
            fhg::util::first_then<std::string> const sep ("", ", ");

            os << prefix;

            BOOST_FOREACH (typename Container::value_type const& x, c)
            {
              os << sep;  print_element (os, x);
            }

            return os << suffix;
          }
        }
      }
    }
  }
}

#endif
