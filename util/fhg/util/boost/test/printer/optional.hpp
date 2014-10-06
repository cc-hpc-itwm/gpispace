// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_BOOST_TEST_PRINTER_OPTIONAL_HPP
#define FHG_UTIL_BOOST_TEST_PRINTER_OPTIONAL_HPP

#include <fhg/util/boost/test.hpp>

#include <boost/optional.hpp>

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  (<typename T>, boost::optional<T>, os, opt)
{
  if (opt)
  {
    os << "Just " << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (opt.get());
  }
  else
  {
    os << "Nothing";
  }
}

#endif
