#include <seda/common.hpp>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>

#ifdef LIBSEDA_ENABLE_LOGGING
#  if defined(LIBSEDA_HAVE_FHGLOG)
#    include <fhglog/Configuration.hpp>
#  elif defined(LIBSEDA_HAVE_LOG4CPP)
#    include <log4cpp/BasicConfigurator.hh>
#    include <log4cpp/Priority.hh>
#  endif
#endif

int
main(int, char **) {
#ifdef LIBSEDA_ENABLE_LOGGING
#  if defined (LIBSEDA_HAVE_FHGLOG)
  ::fhg::log::Configurator::configure();  
#  elif defined (LIBSEDA_HAVE_LOG4CPP)
  ::log4cpp::BasicConfigurator::configure();
  ::log4cpp::Category::setRootPriority(::log4cpp::Priority::DEBUG);
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
