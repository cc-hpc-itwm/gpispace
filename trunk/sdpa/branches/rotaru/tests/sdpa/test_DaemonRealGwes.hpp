#ifndef DAEMON_DUMMY_REAL_TEST_HPP_
#define DAEMON_DUMMY_REAL_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/daemonFSM/BSC/DaemonFSM.hpp"
#include <seda/Strategy.hpp>

namespace sdpa {
		namespace tests {
			class DaemonRealGwesTest: public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::DaemonRealGwesTest );

			  CPPUNIT_TEST( testDaemonFSM_JobFinished );
			  CPPUNIT_TEST( testDaemonFSM_JobFailed );

			  // Caution: GWES blocks the calling thread! Even if the daemon stage uses more than 1 thread
			  // don't use it, wait for AH to fix it
			  // it may happen that the daemon hangs because its threads are blocked by GWES
			  //CPPUNIT_TEST( testDaemonFSM_JobCancelled );
			  //CPPUNIT_TEST( testDaemonFSM_JobCancelled_from_Pending );

			  CPPUNIT_TEST_SUITE_END();

			public:
			  DaemonRealGwesTest();
			  ~DaemonRealGwesTest();
			  void setUp();
			  void tearDown();

			  std::string read_workflow(std::string strFileName);
			protected:
			  void testDaemonFSM_JobFinished();
			  void testDaemonFSM_JobFailed();
			  void testDaemonFSM_JobCancelled();
			  void testDaemonFSM_JobCancelled_from_Pending();

			private:
			  SDPA_DECLARE_LOGGER();
			  sdpa::fsm::bsc::DaemonFSM::ptr_t m_ptrDaemonFSM;
			  sdpa::Sdpa2Gwes* m_ptrSdpa2Gwes;
			  seda::Stage::Ptr m_ptrToMasterStage;
			  seda::Stage::Ptr m_ptrToSlaveStage;
			  seda::Strategy::Ptr m_ptrMasterStrategy;
			  seda::Strategy::Ptr m_ptrSlaveStrategy;
			  std::string m_strWorkflow;
			};
		}
}
#endif
