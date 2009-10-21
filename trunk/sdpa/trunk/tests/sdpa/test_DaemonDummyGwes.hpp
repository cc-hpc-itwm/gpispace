#ifndef DAEMON_DUMMY_GWES_TEST_HPP_
#define DAEMON_DUMMY_GWES_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/daemonFSM/SMC/DaemonFSM.hpp"
#include <seda/Strategy.hpp>

namespace sdpa {
		namespace tests {
			class DaemonDummyGwesTest: public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::DaemonDummyGwesTest );

			  CPPUNIT_TEST( testDaemonFSM_JobFinished );
			  CPPUNIT_TEST( testDaemonFSM_JobFailed );
			  CPPUNIT_TEST( testDaemonFSM_JobCancelled );
			  CPPUNIT_TEST( testDaemonFSM_JobCancelled_from_Pending );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  DaemonDummyGwesTest();
			  ~DaemonDummyGwesTest();
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
			  sdpa::fsm::smc::DaemonFSM::ptr_t m_ptrDaemonFSM;
			  sdpa::Sdpa2Gwes* m_ptrSdpa2Gwes;
			  seda::Stage::Ptr m_ptrOutputStage;
			  seda::Strategy::Ptr m_ptrTestStrategy;
			  std::string m_strWorkflow;
			};
		}
}
#endif
