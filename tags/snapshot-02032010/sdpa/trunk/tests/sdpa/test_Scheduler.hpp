/*
 * =====================================================================================
 *
 *       Filename:  test_Scheduler.hpp
 *
 *    Description:  test the scheduler thread
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
#ifndef SCHEDULER_TEST_HPP_
#define SCHEDULER_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "SchedulerTestImpl.hpp"


namespace sdpa {
		namespace tests {
			class SchedulerTest : public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::SchedulerTest );
			  CPPUNIT_TEST( testSchedulerImpl );
			  CPPUNIT_TEST_SUITE_END();

			public:
			SchedulerTest();
			  ~SchedulerTest();
			  void setUp();
			  void tearDown();

			protected:
			  void testSchedulerImpl();
			private:
			  SDPA_DECLARE_LOGGER();
			};
		}
}
#endif
