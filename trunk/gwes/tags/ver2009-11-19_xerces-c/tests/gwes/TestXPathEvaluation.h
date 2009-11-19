#ifndef TESTXPATHEVALUATION_H_
#define TESTXPATHEVALUATION_H_
//cppunit
#include <cppunit/extensions/HelperMacros.h>
// libxml2
#include <libxml/xpathInternals.h>

namespace gwes {
  namespace tests {
    class XPathEvaluationTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwes::tests::XPathEvaluationTest );
      CPPUNIT_TEST( testXPathEvaluator );
      //CPPUNIT_TEST( testXPathEvaluatorContextCache );
      CPPUNIT_TEST_SUITE_END();

    protected:
      void testXPathEvaluator();
      void testXPathEvaluatorContextCache();
    };
  }
}

void printXmlNodes(xmlNodeSetPtr nodes);

#endif /*TESTXPATHEVALUATION_H_*/
