#ifndef C2D_WITHCOMM_TEST_HPP_
#define C2D_WITHCOMM_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/daemonFSM/DaemonFSM.hpp"
#include <seda/Strategy.hpp>
#include <sdpa/client/ClientApi.hpp>


namespace sdpa {
		namespace tests {
			class C2DWithCommTest: public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::C2DWithCommTest );
			  CPPUNIT_TEST( testUserOrchComm );
			  CPPUNIT_TEST( testUserOrchCommWithGwes );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  C2DWithCommTest();
			  ~C2DWithCommTest();
			  void setUp();
			  void tearDown();

			  std::string read_workflow(std::string strFileName);

			protected:
			  void testUserOrchComm();
			  void testUserOrchCommWithGwes();

			private:
			  SDPA_DECLARE_LOGGER();
			  dsm::DaemonFSM::ptr_t m_ptrOrch;

			  sdpa::Sdpa2Gwes* m_ptrSdpa2GwesOrch;

			  sdpa::client::ClientApi::ptr_t m_ptrUser;

			  std::string m_strWorkflow;

			  int m_nITER;
			  int m_sleep_interval ;
			};
		}
}
#endif
