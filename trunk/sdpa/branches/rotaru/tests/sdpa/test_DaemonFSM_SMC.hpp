#ifndef DAEMON_FSM_SMC_TEST_HPP_
#define DAEMON_FSM_SMC_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemonFSM/SMC/DaemonFSM.hpp"

namespace sdpa {
		namespace tests {
			class DaemonFSMTest_SMC : public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::DaemonFSMTest_SMC );
			  CPPUNIT_TEST( testDaemonFSM_SMC );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  DaemonFSMTest_SMC();
			  ~DaemonFSMTest_SMC();
			  void setUp();
			  void tearDown();

			protected:
			  void testDaemonFSM_SMC();
			private:
			  SDPA_DECLARE_LOGGER();
			  sdpa::fsm::smc::DaemonFSM m_DaemonFSM;
			};
		}
}
#endif
