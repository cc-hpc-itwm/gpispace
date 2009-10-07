#include <seda/common.hpp>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>

#if ENABLE_LOGGING
#include <log4cpp/BasicConfigurator.hh>
#include <log4cpp/Priority.hh>
#include <iostream>
#endif

int
main(int argc, char **argv) {
#if ENABLE_LOGGING
    ::log4cpp::BasicConfigurator::configure();
    ::log4cpp::Category::setRootPriority(::log4cpp::Priority::DEBUG);
#endif
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
