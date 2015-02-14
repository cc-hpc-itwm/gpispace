// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <fhgcom/header.hpp>

#include <fhg/util/boost/test.hpp>

FHG_BOOST_TEST_LOG_VALUE_PRINTER (fhg::com::p2p::address_t, os, address)
{
  os << fhg::com::p2p::to_string (address);
}
