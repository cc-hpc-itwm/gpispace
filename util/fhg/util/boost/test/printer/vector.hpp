// mirko.rahn@itwm.fhg.de

#include <fhg/util/boost/test.hpp>
#include <fhg/util/first_then.hpp>

#include <boost/foreach.hpp>

#include <vector>
#include <string>

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER(<typename T>, std::vector<T>, os, l)
{
  fhg::util::first_then<std::string> const sep ("", ", ");

  os << "vector (";

  BOOST_FOREACH (T const& x, l)
  {
    os << sep << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (x);
  }

  os << ")";
}
