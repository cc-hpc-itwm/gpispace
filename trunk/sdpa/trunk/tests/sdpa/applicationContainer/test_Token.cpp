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
#include <sdpa/Token.hpp>

using namespace sdpa;
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
  Token token(std::string("foo"));

  CPPUNIT_ASSERT_EQUAL(typeid(std::string).name(), token.data().type().name());

  try {
    std::string s(token.as<std::string>());
  } catch (const boost::bad_any_cast &bac) {
    CPPUNIT_ASSERT_MESSAGE("Token contained a string but could not be interpreted as one!", false);
  }

  try {
    int tmp(token.as<int>());
    CPPUNIT_ASSERT_MESSAGE("Token contained a non-integer string and could be casted to int!", false);
  } catch (const boost::bad_any_cast &bac) {
  }
}

void TokenTest::testPropertyPut() {
  Token t;
  std::string expected("bar");
  t.put("foo", expected);
  CPPUNIT_ASSERT_EQUAL(expected, t.get<std::string>("foo"));
}
