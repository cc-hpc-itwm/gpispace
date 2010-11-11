/*
 * =====================================================================================
 *
 *       Filename:  test_Components.hpp
 *
 *    Description:  test all components, each with a real gwes, using a real user client
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
#ifndef ORCHESTRATOR_TEST_HPP_
#define ORCHESTRATOR_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/daemonFSM/DaemonFSM.hpp"
#include <seda/Strategy.hpp>
#include <sdpa/client/ClientApi.hpp>

#include <boost/thread.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>
#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>

using namespace fhg::com;

namespace sdpa {
		namespace tests {
			class TestComponents: public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::TestComponents );

			  CPPUNIT_TEST( testActivityRealWeAllCompActExec );
			  CPPUNIT_TEST( testActivityDummyWeAllCompActExec );
			  CPPUNIT_TEST( testActivityRealWeAllCompAndNreWorkerSpawnedByTest );
			  CPPUNIT_TEST( testActivityRealWeAllCompAndNreWorkerSpawnedByNRE );

			  CPPUNIT_TEST_SUITE_END();

			public:
			  TestComponents();
			  ~TestComponents();
			  void setUp();
			  void tearDown();
			  std::string read_workflow(std::string strFileName);

			protected:
			  void testActivityDummyWeAllCompActExec();
			  void testActivityRealWeAllCompActExec();
			  void testActivityRealWeAllCompAndNreWorkerSpawnedByTest();
			  void testActivityRealWeAllCompAndNreWorkerSpawnedByNRE();

			  void startDaemons(const std::string& workerUrl);
			  void startPcdAndDaemons(const std::string& workerUrl) throw (std::exception);

			private:
			  SDPA_DECLARE_LOGGER();

			  std::string m_strWorkflow;

			  /*io_service_pool pool;
			  tcp_server server;
			  boost::thread thrd;*/

			  fhg::com::io_service_pool *m_ptrPool;
			  fhg::com::kvs::server::kvsd *m_ptrKvsd;
			  fhg::com::tcp_server *m_ptrServ;
			  boost::thread *m_ptrThrd;

			  int m_nITER;
			  int m_sleep_interval ;
			  bool bStarted;
			  int nRemainingTests;
			};
		}
}
#endif
