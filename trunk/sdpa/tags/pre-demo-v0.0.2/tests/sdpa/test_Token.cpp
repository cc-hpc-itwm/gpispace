/***********************************************************************/
/** @file test_Token.cpp
 *
 * $Id:$
 *
 * <short description>
 * <long description>
 *
 *  @author Kai Krueger
 *  @date   2009-05-25
 *  @email  kai.krueger@itwm.fhg.de
 *
 * (C) Fraunhofer ITWM Kaiserslautern
 **/
/*---------------------------------------------------------------------*/

#include <iostream>

#include "test_Token.hpp"
#include <sdpa/wf/Token.hpp>

using namespace sdpa;
using namespace sdpa::wf;
using namespace sdpa::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::TokenTest );

TokenTest::TokenTest() {
}

TokenTest::~TokenTest() {
}

void TokenTest::setUp() {
}

void TokenTest::tearDown() {
}

void TokenTest::testTokenDataTypes() {
  Token token("foo");

  CPPUNIT_ASSERT_EQUAL(std::string("foo"), token.data());

  try {
    std::string s(token.data());
  } catch (const std::exception &) {
    CPPUNIT_ASSERT_MESSAGE("Token contained a string but could not be interpreted as one!", false);
  }

  try {
    token.data_as<int>();
    CPPUNIT_ASSERT_MESSAGE("Token contained a non-integer string but could be casted to one!", false);
  } catch (const std::exception &) {
    // ok
  }
}

void TokenTest::testPropertyPut() {
  Token t;
  const std::string expected("bar");
  t.properties().put("foo", expected);
  CPPUNIT_ASSERT_EQUAL(expected, t.properties().get<std::string>("foo"));
}
