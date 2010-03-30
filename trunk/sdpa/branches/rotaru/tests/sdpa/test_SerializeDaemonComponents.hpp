#ifndef TEST_SERIALIZE_DAEMON_COMPONENTS_HPP_
#define TEST_SERIALIZE_DAEMON_COMPONENTS_HPP_

#include <cppunit/extensions/HelperMacros.h>

namespace sdpa {
  namespace tests {
    class WorkerSerializationTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( sdpa::tests::WorkerSerializationTest);

      CPPUNIT_TEST( testSynchQueueSerialization );
      CPPUNIT_TEST( testWorkerSerialization );
      CPPUNIT_TEST( testSchedulerSerialization );
      CPPUNIT_TEST( testDaemonSerialization );
      CPPUNIT_TEST( testDaemonSerializationWithFSMs );
      CPPUNIT_TEST( testOrchestratorSerialization );
      CPPUNIT_TEST( testAggregatorSerialization );
      CPPUNIT_TEST( testNRESerialization );
      CPPUNIT_TEST( testDummyWorkflowEngineSerialization );
      //CPPUNIT_TEST( testBackupRecover );
      CPPUNIT_TEST_SUITE_END();

    private:
    public:
      WorkerSerializationTest();
      ~WorkerSerializationTest();
      void setUp();
      void tearDown();

    protected:
      void testSynchQueueSerialization();
      void testWorkerSerialization();
      void testSchedulerSerialization();
      void testDaemonSerialization();
      void testDaemonSerializationWithFSMs();
      void testOrchestratorSerialization();
      void testAggregatorSerialization();
      void testDummyWorkflowEngineSerialization();
      void testNRESerialization();
      void testBackupRecover();
    };
  }
}


#endif
