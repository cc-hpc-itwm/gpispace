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

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::ExampleTest );

ExampleTest::ExampleTest() {
}

ExampleTest::~ExampleTest() {
}

void ExampleTest::setUp() {
  //createFixtureFile(std::string("fixture"), std::string("bar"),
  //                  std::string("djiowe\ndwejodjweio"));
}

void ExampleTest::tearDown() {
  //deleteFixtureDirectory(std::string("fixture"));
}

void ExampleTest::testExampleBase() {
  CPPUNIT_ASSERT_EQUAL(1,1);
}
