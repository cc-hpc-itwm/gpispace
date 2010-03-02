#ifndef TESTGWES_H_
#define TESTGWES_H_
//cppunit
#include <cppunit/extensions/HelperMacros.h>
//gwes
#include <gwes/GWES.h>

namespace gwes {
  namespace tests {
    class GWESTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwes::tests::GWESTest );
      CPPUNIT_TEST( testGWES );
      CPPUNIT_TEST_SUITE_END();

      public:
      GWESTest();
      ~GWESTest();
      
    protected:
      void testGWES() ;
      
    };
  }
}

#endif /*TESTGWES_H_*/
