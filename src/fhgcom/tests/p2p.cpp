// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhgcom/address.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
namespace com
{
BOOST_AUTO_TEST_CASE ( test_address_translation_equal )
{
  BOOST_CHECK (  fhg::com::p2p::address_t (host_t {"name"}, port_t {1})
              == fhg::com::p2p::address_t (host_t {"name"}, port_t {1})
              );
}

BOOST_AUTO_TEST_CASE ( test_address_translation_unique )
{
  BOOST_CHECK (  fhg::com::p2p::address_t (host_t {"name"}, port_t {1})
              != fhg::com::p2p::address_t (host_t {"name"}, port_t {2})
              );
}
}
}
