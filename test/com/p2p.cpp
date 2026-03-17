// Copyright (C) 2010,2014-2016,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/com/address.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>


namespace gspc::com
{
BOOST_AUTO_TEST_CASE ( test_address_translation_equal )
{
  BOOST_CHECK (  gspc::com::p2p::address_t (host_t {"name"}, port_t {1})
              == gspc::com::p2p::address_t (host_t {"name"}, port_t {1})
              );
}

BOOST_AUTO_TEST_CASE ( test_address_translation_unique )
{
  BOOST_CHECK (  gspc::com::p2p::address_t (host_t {"name"}, port_t {1})
              != gspc::com::p2p::address_t (host_t {"name"}, port_t {2})
              );
}
}
