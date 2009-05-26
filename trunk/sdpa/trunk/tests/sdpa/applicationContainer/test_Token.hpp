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

#include <cppunit/extensions/HelperMacros.h>

namespace sdpa {
  namespace tests {
    class TokenTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( sdpa::tests::TokenTest);
      CPPUNIT_TEST( testTokenDataTypes );
      CPPUNIT_TEST( testPropertyPut );
      CPPUNIT_TEST_SUITE_END();

    private:
    public:
      TokenTest();
      ~TokenTest();
      void setUp();
      void tearDown();

    protected:
      void testTokenDataTypes();
      void testPropertyPut();
    };
  }
}
