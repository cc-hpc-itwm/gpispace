/***********************************************************************/
/** @file test_Example.cpp
 *
 * $Id:$
 *
 * <short description>
 * <long description>
 *
 *  @author Kai Krueger
 *  @date   2009-05-14
 *  @email  kai.krueger@itwm.fhg.de
 *
 * (C) Fraunhofer ITWM Kaiserslautern
 **/
/*---------------------------------------------------------------------*/

#include "test_Example.hpp"

using namespace sdpa::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::ExampleTest2 );

ExampleTest2::ExampleTest2() {
}

ExampleTest2::~ExampleTest2() {
}

void ExampleTest2::setUp() {
  //createFixtureFile(std::string("fixture"), std::string("bar"),
  //                  std::string("djiowe\ndwejodjweio"));
}

void ExampleTest2::tearDown() {
  //deleteFixtureDirectory(std::string("fixture"));
}

void ExampleTest2::testExampleBase() {
  CPPUNIT_ASSERT_EQUAL(1,1);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Force failure",0,0 );
}
