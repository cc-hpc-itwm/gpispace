// mirko.rahn@itwm.fhg.de

#pragma once

#include <fhg/util/boost/test.hpp>
#include <fhg/util/boost/test/printer/container.hpp>

#include <list>

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER(<typename T>, std::list<T>, os, l)
{
  fhg::util::boost::test::printer::container (os, l, "list (", ")");
}
