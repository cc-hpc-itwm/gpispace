// mirko.rahn@itwm.fhg.de

#ifndef FHG_UTIL_BOOST_TEST_PRINTER_MAP_HPP
#define FHG_UTIL_BOOST_TEST_PRINTER_MAP_HPP

#include <fhg/util/boost/test.hpp>
#include <fhg/util/boost/test/printer/container.hpp>

#include <boost/preprocessor/punctuation/comma.hpp>

#include <map>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace test
      {
        namespace printer
        {
          template<typename K, typename V>
          class print_map_element
          {
          public:
            void operator() ( std::ostream& os
                            , typename std::map<K, V>::value_type const& x
                            ) const
            {
              os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (x.first)
                 << " -> "
                 << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (x.second);
            }
          };
        }
      }
    }
  }
}

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  ( <typename K BOOST_PP_COMMA() typename V>
  , std::map<K BOOST_PP_COMMA() V>
  , os
  , m
  )
{
  fhg::util::boost::test::printer::container
    ( os, m, "map [", "]"
    , fhg::util::boost::test::printer::print_map_element<K, V>()
    );
}

#endif
