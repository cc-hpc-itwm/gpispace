// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_BOOST_TEST_PRINTER_OPTIONAL_HPP
#define FHG_UTIL_BOOST_TEST_PRINTER_OPTIONAL_HPP

#include <fhg/util/boost/test.hpp>

#include <chrono>

FHG_BOOST_TEST_LOG_VALUE_PRINTER (std::chrono::nanoseconds, os, duration)
{
  os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (duration.count()) << "ns";
}
FHG_BOOST_TEST_LOG_VALUE_PRINTER (std::chrono::microseconds, os, duration)
{
  os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (duration.count()) << "µs";
}
FHG_BOOST_TEST_LOG_VALUE_PRINTER (std::chrono::milliseconds, os, duration)
{
  os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (duration.count()) << "ms";
}
FHG_BOOST_TEST_LOG_VALUE_PRINTER (std::chrono::seconds, os, duration)
{
  os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (duration.count()) << "s";
}
FHG_BOOST_TEST_LOG_VALUE_PRINTER (std::chrono::minutes, os, duration)
{
  os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (duration.count()) << "min";
}
FHG_BOOST_TEST_LOG_VALUE_PRINTER (std::chrono::hours, os, duration)
{
  os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (duration.count()) << "hr";
}

#endif
