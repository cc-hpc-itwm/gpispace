#pragma once

#include <fhgcom/header.hpp>

#include <util-generic/testing/printer/generic.hpp>

FHG_BOOST_TEST_LOG_VALUE_PRINTER (fhg::com::p2p::address_t, os, address)
{
  os << fhg::com::p2p::to_string (address);
}
