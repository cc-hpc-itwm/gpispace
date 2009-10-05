// cppunit includes
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestFailure.h>
#include <cppunit/Test.h>

#include <cppunit/XmlOutputterHook.h>
#include <cppunit/tools/XmlElement.h>
#include <cppunit/tools/StringTools.h>
#include <cppunit/tools/XmlDocument.h>
#include <cppunit/TestFailure.h>
#include <cppunit/SourceLine.h>
#include <cppunit/Exception.h>
#include <cppunit/Message.h>

#include <sdpa/LoggingConfigurator.hpp>
#include <tests/sdpa/Suite.hpp>

int main(int /* argc */, char ** /* argv */)
{
  sdpa::logging::Configurator::configure();

  // Informiert Test-Listener ueber Testresultate
  CPPUNIT_NS::TestResult                   testresult;
  CPPUNIT_NS::TestResultCollector collectedresults;
  testresult.addListener (&collectedresults);

  CPPUNIT_NS :: TestRunner runner;
  runner.addTest ( sdpa::tests::Suite::suite() );

  std::cout << "running testsuite" << std::endl;

  runner.run (testresult);

  std::cout << "running testsuite  done" << std::endl;
  // print and save results
  std::ofstream outStream("out.xml");
  CPPUNIT_NS::XmlOutputter xmloutputter (&collectedresults, outStream);
  xmloutputter.write ();

  return collectedresults.wasSuccessful () ? 0 : 1;
}
