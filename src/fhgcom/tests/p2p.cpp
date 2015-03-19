#define BOOST_TEST_MODULE P2PTest
#include <boost/test/unit_test.hpp>

#include <fhgcom/header.hpp>
#include <fhgcom/tests/address_printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE ( test_address )
{
  BOOST_CHECK_EQUAL (16ul, sizeof (fhg::com::p2p::address_t));

  BOOST_CHECK_EQUAL (40ul, sizeof (fhg::com::p2p::header_t));
}

BOOST_AUTO_TEST_CASE ( test_address_translation_equal )
{
  BOOST_CHECK_EQUAL ( fhg::com::p2p::address_t ("name-1")
                    , fhg::com::p2p::address_t ("name-1")
                    );
}

BOOST_AUTO_TEST_CASE ( test_address_translation_unique )
{
  BOOST_CHECK_NE ( fhg::com::p2p::address_t ("name-1")
                 , fhg::com::p2p::address_t ("name-2")
                 );
}
