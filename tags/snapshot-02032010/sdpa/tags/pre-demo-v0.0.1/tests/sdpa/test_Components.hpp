#ifndef ORCHESTRATOR_TEST_HPP_
#define ORCHESTRATOR_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/daemonFSM/DaemonFSM.hpp"
#include <seda/Strategy.hpp>
#include <sdpa/client/ClientApi.hpp>


namespace sdpa {
		namespace tests {
			class TestComponents: public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::TestComponents );
			  CPPUNIT_TEST( testComponents );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  TestComponents();
			  ~TestComponents();
			  void setUp();
			  void tearDown();

			  std::string read_workflow(std::string strFileName);

			protected:
			  void testComponents();

			private:
			  SDPA_DECLARE_LOGGER();

			  sdpa::client::ClientApi::ptr_t m_ptrUser;
			  std::string m_strWorkflow;

			  int m_nITER;
			  int m_sleep_interval ;
			};
		}
}
#endif
