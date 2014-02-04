// mirko.rahn@itwm.fhg.de

#ifndef FHG_UTIL_BOOST_TEST_PRINTER_SET_HPP
#define FHG_UTIL_BOOST_TEST_PRINTER_SET_HPP

#include <fhg/util/boost/test.hpp>
#include <fhg/util/boost/test/printer/container.hpp>

#include <set>

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER(<typename T>, std::set<T>, os, s)
{
  fhg::util::boost::test::printer::container (os, s, "set {", "}");
}

#endif
