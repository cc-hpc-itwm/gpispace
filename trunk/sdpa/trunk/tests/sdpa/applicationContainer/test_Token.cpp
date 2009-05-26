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
  //createFixtureFile(std::string("fixture"), std::string("bar"),
  //                  std::string("djiowe\ndwejodjweio"));
}

void TokenTest::tearDown() {
  //deleteFixtureDirectory(std::string("fixture"));
}

void TokenTest::testTokenBase() {
  CPPUNIT_ASSERT_EQUAL(1,1);

  DataToken<int> intToken(42); // creation of a Token
  Token  &tok(intToken); // we got a generic Token reference from someone

  int value = tok.as<int>(); // we want to use it as an integer (DANGER, we loose any type safety here)

  try {
    char  f = tok.as<char>(); // assert should crash
    std::cout << f << std::endl;
  } catch (...) {
    std::cout << "blah" << std::endl;
  }

  //if (t) {
     // t has been initialized?
  //}

  //ControlToken bToken1(true);
  //ControlToken bToken2(false);
  //IntToken nToken(32);
  
  //CPPUNIT_ASSERT_EQUAL(nToken.as<int>(), 32);
  
  //if(bToken1.as<bool>()) CPPUNIT_ASSERT_EQUAL(1,1);
  //else CPPUNIT_ASSERT_EQUAL(1,0);
  
}
