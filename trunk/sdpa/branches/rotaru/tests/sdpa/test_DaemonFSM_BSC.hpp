#ifndef DAEMON_FSM_BSC_TEST_HPP_
#define DAEMON_FSM_BSC_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/daemonFSM/BSC/DaemonFSM.hpp"
#include <sdpa/wf/Gwes2Sdpa.hpp>
#include <seda/Strategy.hpp>

namespace sdpa {
		namespace tests {
			class DaemonFSMTest_BSC : public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE(sdpa::tests::DaemonFSMTest_BSC);
			  CPPUNIT_TEST(testDaemonFSM_BSC);
			  CPPUNIT_TEST_SUITE_END();

			public:
			  DaemonFSMTest_BSC();
			  ~DaemonFSMTest_BSC();
			  void setUp();
			  void tearDown();

			protected:
			  void testDaemonFSM_BSC();
			private:
			  SDPA_DECLARE_LOGGER();
			  shared_ptr<sdpa::fsm::bsc::DaemonFSM> m_ptrDaemonFSM;
			  sdpa::wf::Sdpa2Gwes::ptr_t m_ptrSdpa2Gwes;
			  seda::Stage::Ptr m_ptrOutputStage;
			  seda::Strategy::Ptr ptrTestStrategy;
			};
		}
}
#endif
