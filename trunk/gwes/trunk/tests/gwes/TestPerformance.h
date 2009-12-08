#ifndef TESTPERFORMANCE_H_
#define TESTPERFORMANCE_H_
//cppunit
#include <cppunit/extensions/HelperMacros.h>
//gwes
#include <gwes/GWES.h>
//fhglog
#include <fhglog/fhglog.hpp>

namespace gwes {
  namespace tests {
    class TestPerformance : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwes::tests::TestPerformance );
      CPPUNIT_TEST( testWorkflowControlLoop ); 
      CPPUNIT_TEST( testManySimpleWorkflows );
      CPPUNIT_TEST( testManyConcurrentWorkflows );
      CPPUNIT_TEST_SUITE_END();

      public:
      TestPerformance();
      ~TestPerformance();
      
    protected:
      void testWorkflowControlLoop();
      void testManySimpleWorkflows(); 
      void testManyConcurrentWorkflows()
      
    private:
      gwdl::Workflow::ptr_t _testWorkflow(std::string workflowfn, gwes::GWES &gwes);
      std::string _executeWorkflow(gwdl::Workflow::ptr_t wfP, gwes::GWES &gwes);
      WorkflowHandler::status_t _monitorWorkflow(WorkflowHandler::status_t oldStatus, WorkflowHandler* wfhP);
      void _loggerShutup();
      void _loggerAsBefore();
      static gwes::GWES m_gwes;
      fhg::log::LogLevel _oldGwesLevel;
      fhg::log::LogLevel _oldGwdlLevel;
    };
  }
}

#endif /*TESTPERFORMANCE_H_*/
