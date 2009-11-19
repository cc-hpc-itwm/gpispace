/*
 * =====================================================================================
 *
 *       Filename:  test_D2D2DDummyGwes.hpp
 *
 *    Description:  test 3 generic daemons, each with a dummy gwes
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
#ifndef D2D2D_DUMMY_GWES_TEST_HPP_
#define D2D2D_DUMMY_GWES_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/daemonFSM/DaemonFSM.hpp"
#include <seda/Strategy.hpp>

namespace sdpa {
		namespace tests {
			class D2D2DDummyGwesTest: public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::D2D2DDummyGwesTest );

			  CPPUNIT_TEST( testDaemonFSM_JobFinished );
			  CPPUNIT_TEST( testDaemonFSM_JobFailed );
			  CPPUNIT_TEST( testDaemonFSM_JobCancelled );
			  CPPUNIT_TEST( testDaemonFSM_JobCancelled_from_Pending );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  D2D2DDummyGwesTest();
			  ~D2D2DDummyGwesTest();
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
			  dsm::DaemonFSM::ptr_t m_ptrOrch;
			  dsm::DaemonFSM::ptr_t m_ptrAgg;
			  dsm::DaemonFSM::ptr_t m_ptrNRE;

			  sdpa::Sdpa2Gwes* m_ptrSdpa2GwesOrch;
			  sdpa::Sdpa2Gwes* m_ptrSdpa2GwesAgg;

			  seda::Stage::Ptr m_ptrToUserStage;
			  seda::Strategy::Ptr m_ptrUserStrategy;

			  std::string m_strWorkflow;
			  int m_nITER;
			  int m_sleep_interval ;
			};
		}
}
#endif
