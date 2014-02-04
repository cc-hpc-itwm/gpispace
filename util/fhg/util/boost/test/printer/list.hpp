// mirko.rahn@itwm.fhg.de

#ifndef FHG_UTIL_BOOST_TEST_PRINTER_LIST_HPP
#define FHG_UTIL_BOOST_TEST_PRINTER_LIST_HPP

#include <fhg/util/boost/test.hpp>
#include <fhg/util/boost/test/printer/container.hpp>

#include <list>

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER(<typename T>, std::list<T>, os, l)
{
  fhg::util::boost::test::printer::container (os, l, "list (", ")");
}

#endif
