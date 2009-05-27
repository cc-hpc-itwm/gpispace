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
#include <sdpa/applicationContainer/Token.hpp>

using namespace sdpa::tests;
using namespace sdpa::appcontainer;

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
  CPPUNIT_ASSERT_EQUAL(42, value);
  
  try {
    // assert should crash
    char f = tok.as<char>();
    //CPPUNIT_ASSERT_THROW((tok.as<char>()), std::Exception);
    CPPUNIT_FAIL("Did not throw exception");
    std::cout << f << std::endl;
  } catch (...) {
    std::cout << "blah" << std::endl;
  }
  
  //std::cout << t.as<float>() << std::endl;
  //if (t) {
     // t has been initialized?
  //}

  //ControlToken bToken1(true);
  //ControlToken bToken2(false);
  //IntToken nToken(32);
  
  CPPUNIT_ASSERT_EQUAL(tok.as<int>(), 42);
  
  //if(bToken1.as<bool>()) CPPUNIT_ASSERT_EQUAL(1,1);
  //else CPPUNIT_ASSERT_EQUAL(1,0);
  
}
void TokenTest::testTokenCopy() {
  DataToken<int> intToken(42); // creation of a Token
  Token  &tok(intToken); // we got a generic Token reference from someone
  float fExpected = 123456.0;
  
  Token t = tok;
  {
    DataToken<int> t1( 0xdeadbeef);
    DataToken<float> t2(fExpected);
    DataToken<int>t3(0xdeadbeef);
    t = t2;
  }
  {
    DataToken<int>t3(0xdeadbeef);
    DataToken<int> t1( 0xdeadbeef);
    DataToken<float> t2(120435.0);
  }      
  CPPUNIT_ASSERT_EQUAL(fExpected,  t.as<float>());
}
