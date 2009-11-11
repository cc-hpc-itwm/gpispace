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
			class DaemonsWithCommTest: public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::DaemonsWithCommTest );

			  //CPPUNIT_TEST( testUserOrchCommDummyGwes );
			  CPPUNIT_TEST( testUserOrchCommRealGwes );

			  /*CPPUNIT_TEST( testUserOrchAggCommDummyGwes );
			  CPPUNIT_TEST( testUserOrchAggCommRealGwes );
			  CPPUNIT_TEST( testUserOrchAggNRECommDummyGwes );
			  CPPUNIT_TEST( testUserOrchAggNRECommRealGwes );*/
			  CPPUNIT_TEST_SUITE_END();

			public:
			  DaemonsWithCommTest();
			  ~DaemonsWithCommTest();
			  void setUp();
			  void tearDown();

			  std::string read_workflow(std::string strFileName);

			protected:
			  void testUserOrchCommDummyGwes();
			  void testUserOrchCommRealGwes();
			  void testUserOrchAggCommDummyGwes();
			  void testUserOrchAggCommRealGwes();
			  void testUserOrchAggNRECommDummyGwes();
			  void testUserOrchAggNRECommRealGwes();

			private:
			  SDPA_DECLARE_LOGGER();
			  dsm::DaemonFSM::ptr_t m_ptrOrch;
			  dsm::DaemonFSM::ptr_t m_ptrAgg;
			  dsm::DaemonFSM::ptr_t m_ptrNRE;

			  sdpa::Sdpa2Gwes* m_ptrSdpa2GwesOrch;
			  sdpa::Sdpa2Gwes* m_ptrSdpa2GwesAgg;

			  sdpa::client::ClientApi::ptr_t m_ptrUser;
			  std::string m_strWorkflow;

			  int m_nITER;
			  int m_sleep_interval ;
			};
		}
}
#endif
