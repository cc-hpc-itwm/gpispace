#ifndef D2D_DUMMY_GWES_TEST_HPP_
#define D2D_DUMMY_GWES_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/daemonFSM/BSC/DaemonFSM.hpp"
#include <seda/Strategy.hpp>

namespace sdpa {
		namespace tests {
			class D2DDummyGwesTest: public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::D2DDummyGwesTest );

			  CPPUNIT_TEST( testDaemonFSM_JobFinished );
			  CPPUNIT_TEST( testDaemonFSM_JobFailed );
			  CPPUNIT_TEST( testDaemonFSM_JobCancelled );
			  CPPUNIT_TEST( testDaemonFSM_JobCancelled_from_Pending );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  D2DDummyGwesTest();
			  ~D2DDummyGwesTest();
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
			  sdpa::fsm::bsc::DaemonFSM::ptr_t m_ptrOrch;
			  sdpa::fsm::bsc::DaemonFSM::ptr_t m_ptrAgg;

			  sdpa::Sdpa2Gwes* m_ptrSdpa2GwesOrch;
			  sdpa::Sdpa2Gwes* m_ptrSdpa2GwesAgg;

			  seda::Stage::Ptr m_ptrToUserStage;
			  seda::Stage::Ptr m_ptrToNreStage;

			  seda::Strategy::Ptr m_ptrUserStrategy;
			  seda::Strategy::Ptr m_ptrNreStrategy;

			  std::string m_strWorkflow;
			};
		}
}
#endif
