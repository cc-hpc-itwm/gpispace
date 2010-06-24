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


namespace sdpa {
		namespace tests {
			class TestComponents: public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::TestComponents );

			  //obsolete
			  //CPPUNIT_TEST( testCompDummyGwesAndFakeFvmPC );
			  //CPPUNIT_TEST( testComponentsDummyGwesNoFvmPC );

			  CPPUNIT_TEST( testActivityDummyWeAllCompAndNreWorker );
			  CPPUNIT_TEST( testActivityRealWeAllCompAndActExec );
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
			  void testAny();
			  void testCompDummyGwesAndFakeFvmPC();
			  void testComponentsDummyGwesNoFvmPC();
			  void testActivityDummyWeAllCompAndNreWorker();

			  void testActivityRealWeAllCompAndActExec();

			  void testActivityRealWeAllCompAndNreWorkerSpawnedByTest();
			  void testActivityRealWeAllCompAndNreWorkerSpawnedByNRE();

			  void startDaemons(const std::string& workerUrl);
			  void startPcdAndDaemons(const std::string& workerUrl) throw (std::exception);

			private:
			  SDPA_DECLARE_LOGGER();

			  sdpa::client::ClientApi::ptr_t m_ptrCli;
			  std::string m_strWorkflow;

			  int m_nITER;
			  int m_sleep_interval ;
			};
		}
}
#endif
