// mirko.rahn@itwm.fhg.de

#include <fhg/util/boost/test.hpp>
#include <fhg/util/first_then.hpp>

#include <boost/foreach.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>

#include <map>
#include <string>

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  ( <typename K BOOST_PP_COMMA() typename V>
  , std::map<K BOOST_PP_COMMA() V>
  , os
  , m
  )
{
  fhg::util::first_then<std::string> const sep ("", ", ");

  os << "map [";

  BOOST_FOREACH ( typename std::map<K BOOST_PP_COMMA() V>::value_type const& kv
                , m
                )
  {
    os << sep << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (kv.first)
       << " -> " << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (kv.second);
  }

  os << "]";
}
