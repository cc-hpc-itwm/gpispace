/*
 * =====================================================================================
 *
 *       Filename:  test_C2D2D2DDummyGwes.hpp
 *
 *    Description:  test 3 generic daemons, each with a dummy gwes, using a real user client
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef C2D2D2D_DUMMY_GWES_TEST_HPP_
#define C2D2D2D_DUMMY_GWES_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/daemonFSM/DaemonFSM.hpp"
#include <seda/Strategy.hpp>
#include <sdpa/client/ClientApi.hpp>


namespace sdpa {
		namespace tests {
			class C2D2D2DDummyGwesTest: public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::C2D2D2DDummyGwesTest );

			  CPPUNIT_TEST( testDaemonFSM_JobFinished );
			  CPPUNIT_TEST( testDaemonFSM_JobFailed );
			  //CPPUNIT_TEST( testDaemonFSM_JobCancelled );
			  CPPUNIT_TEST( testDaemonFSM_JobFinished_WithGwes );
			  CPPUNIT_TEST( testDaemonFSM_JobFailed_WithGwes );
			  //CPPUNIT_TEST( testDaemonFSM_JobCancelled_WithGwes );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  C2D2D2DDummyGwesTest();
			  ~C2D2D2DDummyGwesTest();
			  void setUp();
			  void tearDown();

			  std::string read_workflow(std::string strFileName);

			protected:
			  void testDaemonFSM_JobFinished();
			  void testDaemonFSM_JobFailed();
			  void testDaemonFSM_JobCancelled();

			  void testDaemonFSM_JobFinished_WithGwes();
			  void testDaemonFSM_JobFailed_WithGwes();
			  void testDaemonFSM_JobCancelled_WithGwes();

			private:
			  SDPA_DECLARE_LOGGER();
			  dsm::DaemonFSM::ptr_t m_ptrOrch;
			  dsm::DaemonFSM::ptr_t m_ptrAgg;
			  dsm::DaemonFSM::ptr_t m_ptrNRE;

			  sdpa::Sdpa2Gwes* m_ptrSdpa2GwesOrch;
			  sdpa::Sdpa2Gwes* m_ptrSdpa2GwesAgg;
			  sdpa::Sdpa2Gwes* m_ptrSdpa2GwesNRE;

			  sdpa::client::ClientApi::ptr_t m_ptrUser;

			  std::string m_strWorkflow;

			  int m_nITER;
			  int m_sleep_interval ;
			};
		}
}
#endif
