#ifndef TESTWORKFLOWS_H_
#define TESTWORKFLOWS_H_
//cppunit
#include <cppunit/extensions/HelperMacros.h>
//gwes
#include <gwes/GWES.h>

namespace gwes {
  namespace tests {
    class TestWorkflows : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwes::tests::TestWorkflows );
      CPPUNIT_TEST( testWorkflowSimpleGwdl );
      CPPUNIT_TEST( testWorkflowSplitToken );
      CPPUNIT_TEST( testWorkflowExclusiveChoice );
      CPPUNIT_TEST( testWorkflowConditionTest );
      CPPUNIT_TEST( testWorkflowControlLoop ); 
      // Test Currently deactiviated // CPPUNIT_TEST( testWorkflowConcatenateIt );
      // Test Currently deactiviated // CPPUNIT_TEST( testWorkflowConcatenateItFail );
      CPPUNIT_TEST( testWorkflowPstm0 ); 
      CPPUNIT_TEST_SUITE_END();

      public:
      TestWorkflows();
      ~TestWorkflows();
      
    protected:
      void testGWES() ;
      void testWorkflowSimpleGwdl();
      void testWorkflowSplitToken();
      void testWorkflowExclusiveChoice();
      void testWorkflowConditionTest();
      void testWorkflowControlLoop(); 
      void testWorkflowConcatenateIt();
      void testWorkflowConcatenateItFail();
      void testWorkflowPstm0(); 
      
    private:
      gwdl::Workflow& _testWorkflow(std::string workflowfn, gwes::GWES &gwes);
      WorkflowHandler::status_t _monitorWorkflow(WorkflowHandler::status_t oldStatus, WorkflowHandler* wfhP);
      gwes::GWES m_gwes; 
    };
  }
}

#endif /*TESTWORKFLOWS_H_*/
