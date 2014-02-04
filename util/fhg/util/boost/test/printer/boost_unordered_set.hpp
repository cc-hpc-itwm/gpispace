// mirko.rahn@itwm.fhg.de

#ifndef FHG_UTIL_BOOST_TEST_PRINTER_BOOST_UNORDERED_MAP_HPP
#define FHG_UTIL_BOOST_TEST_PRINTER_BOOST_UNORDERED_MAP_HPP

#include <fhg/util/boost/test.hpp>
#include <fhg/util/boost/test/printer/container.hpp>

#include <boost/unordered_set.hpp>

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  (<typename T>, boost::unordered_set<T>, os, s)
{
  fhg::util::boost::test::printer::container
    (os, s, "boost_unordered_set {", "}");
}

#endif
