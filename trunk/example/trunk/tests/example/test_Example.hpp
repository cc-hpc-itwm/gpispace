/***********************************************************************/
/** @file test_Example.hpp
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

#include <cppunit/extensions/HelperMacros.h>

namespace sdpa {
  namespace tests {
    class ExampleTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( sdpa::tests::ExampleTest );
      CPPUNIT_TEST( testExampleBase );
      CPPUNIT_TEST_SUITE_END();

    private:
    public:
      ExampleTest();
      ~ExampleTest();
      void setUp();
      void tearDown();

    protected:
      void testExampleBase();
    };
  }
}
