#ifndef DAEMON_FSM_BSC_TEST_HPP_
#define DAEMON_FSM_BSC_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/logging.hpp"
#include "sdpa/daemonFSM/BSC/DaemonFSM.hpp"

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
			  sdpa::fsm::bsc::DaemonFSM m_DaemonFSM;
			};
		}
}
#endif
