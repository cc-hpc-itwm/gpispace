#ifndef DAEMON_FSM_TEST_HPP_
#define DAEMON_FSM_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/daemonFSM/SMC/DaemonFSM.hpp"
#include <sdpa/wf/Gwes2Sdpa.hpp>
#include <seda/Strategy.hpp>

using namespace sdpa::fsm::smc;

namespace sdpa {
		namespace tests {
			class DaemonFSMTest: public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::DaemonFSMTest );
			  //CPPUNIT_TEST( testDaemonFSM_JobFinished );
			  //CPPUNIT_TEST( testDaemonFSM_JobFailed );
			  CPPUNIT_TEST( testDaemonFSM_JobCancelled );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  DaemonFSMTest();
			  ~DaemonFSMTest();
			  void setUp();
			  void tearDown();

			protected:
			  void testDaemonFSM_JobFinished();
			  void testDaemonFSM_JobFailed();
			  void testDaemonFSM_JobCancelled();

			private:
			  SDPA_DECLARE_LOGGER();
			  DaemonFSM::ptr_t m_ptrDaemonFSM;
			  sdpa::wf::Sdpa2Gwes::ptr_t m_ptrSdpa2Gwes;
			  seda::Stage::Ptr m_ptrOutputStage;
			  seda::Strategy::Ptr m_ptrTestStrategy;
			};
		}
}
#endif
