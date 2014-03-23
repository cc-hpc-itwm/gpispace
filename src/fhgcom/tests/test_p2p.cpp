#define BOOST_TEST_MODULE P2PTest
#include <boost/test/unit_test.hpp>

#include <fhgcom/header.hpp>
#include <fhgcom/tests/address_printer.hpp>

#include <iostream>

BOOST_AUTO_TEST_CASE ( test_address )
{
  fhg::com::p2p::address_t a1;
  BOOST_CHECK_EQUAL (16ul, sizeof (a1));

  fhg::com::p2p::header_t hdr;
  BOOST_CHECK_EQUAL (40ul, sizeof (hdr));
}

BOOST_AUTO_TEST_CASE ( test_address_translation_equal )
{
  fhg::com::p2p::address_t a1 ("name-1");
  fhg::com::p2p::address_t a2 ("name-1");

  BOOST_CHECK_EQUAL ( a1, a2 );
}

BOOST_AUTO_TEST_CASE ( test_address_translation_unique )
{
  fhg::com::p2p::address_t a1 ("name-1");
  fhg::com::p2p::address_t a2 ("name-2");

  BOOST_CHECK_NE ( a1, a2 );
}
