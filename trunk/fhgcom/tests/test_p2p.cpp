#define BOOST_TEST_MODULE P2PTest
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fhgcom/header.hpp>

BOOST_AUTO_TEST_CASE ( test_address )
{
  using namespace fhg::com::p2p;
  address_t a1;
  BOOST_CHECK_EQUAL (16ul, sizeof (a1));

  header_t hdr;
  BOOST_CHECK_EQUAL (40ul, sizeof (hdr));
}

BOOST_AUTO_TEST_CASE ( test_address_translation_equal )
{
  using namespace fhg::com::p2p;
  address_t a1 ("name-1");
  address_t a2 ("name-1");

  BOOST_CHECK_EQUAL ( a1, a2 );
}

BOOST_AUTO_TEST_CASE ( test_address_translation_unique )
{
  using namespace fhg::com::p2p;
  address_t a1 ("name-1");
  address_t a2 ("name-2");

  BOOST_CHECK ( a1 != a2 );
}
