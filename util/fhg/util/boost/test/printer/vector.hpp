// mirko.rahn@itwm.fhg.de

#ifndef FHG_UTIL_BOOST_TEST_PRINTER_VECTOR_HPP
#define FHG_UTIL_BOOST_TEST_PRINTER_VECTOR_HPP

#include <fhg/util/boost/test.hpp>
#include <fhg/util/boost/test/printer/container.hpp>

#include <vector>

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER(<typename T>, std::vector<T>, os, v)
{
  fhg::util::boost::test::printer::container (os, v, "vector (", ")");
}

#endif
