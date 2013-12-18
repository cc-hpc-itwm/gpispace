#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>

int
main(int, char **) {
  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest( registry.makeTest() );
  CppUnit::CompilerOutputter *outputter =
    new CppUnit::CompilerOutputter(&runner.result(), std::cout);
  outputter->setLocationFormat("%p(%l) : ");
  //outputter->setWrapColumn(19);
  outputter->setNoWrap();
  runner.setOutputter(outputter);
  bool wasSuccessful = runner.run("",
				  false, // doWait
				  true,  // doPrintResult
				  true   // doPrintProgress
				  );
  return !wasSuccessful;
}
