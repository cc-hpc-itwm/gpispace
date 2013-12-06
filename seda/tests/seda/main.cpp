#include <seda/common.hpp>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>

#ifdef SEDA_ENABLE_LOGGING
#  if defined(SEDA_HAVE_FHGLOG)
#    include <fhglog/Configuration.hpp>
#  endif
#endif

int
main(int, char **) {
#ifdef SEDA_ENABLE_LOGGING
#  if defined (SEDA_HAVE_FHGLOG)
  ::fhg::log::Configurator::configure();
#  endif
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
