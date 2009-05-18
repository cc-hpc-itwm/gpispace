/***********************************************************************/
/** @file main.cpp
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

/*---------------------------------------------------------------------*
 * System headers
 *
 *---------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------*
 * Local headers
 *
 *---------------------------------------------------------------------*/

int main(
  int argc,
  char **argv
  )
{
  // Informiert Test-Listener ueber Testresultate
  CPPUNIT_NS::TestResult                   testresult;
  CPPUNIT_NS::TestResultCollector collectedresults;
  testresult.addListener (&collectedresults);

  // Test-Suite ueber die Registry im Test-Runner einfuegen
  CPPUNIT_NS :: TestRunner runner;
  runner.addTest (CPPUNIT_NS::TestFactoryRegistry :: getRegistry ().makeTest ());

  //CPPUNIT_NS::CompilerOutputter *outputter = new CPPUNIT_NS::CompilerOutputter(&runner.result(), std::cout);
  //outputter->setLocationFormat("%p(%l) : ");
  //outputter->setNoWrap();
  //runner.setOutputter(outputter);

  std::cout << "running testsuite" << std::endl;

  runner.run (testresult);

  // print and save results
  std::ofstream outStream("out.xml");
  //outStream= new std::ofstream("out.xml", std::ios::app );
  CPPUNIT_NS::XmlOutputter xmloutputter (&collectedresults, outStream);
  //CPPUNIT_NS::XmlOutputter xmloutputter (&collectedresults, std::cout);
  xmloutputter.write ();

  std::ofstream outStream2("out2.xml");
  CPPUNIT_NS::XmlOutputter xmloutputter2 (&collectedresults, outStream2);
  xmloutputter2.write ();

  // text mode
  //CPPUNIT_NS::CompilerOutputter compileroutputter (&collectedresults, std::cout);
  //compileroutputter.write ();


  //
  return collectedresults.wasSuccessful () ? 0 : 1;
}
