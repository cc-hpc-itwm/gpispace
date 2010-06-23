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
#ifndef LB_TEST_HPP_
#define LB_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"

namespace sdpa {
		namespace tests {
			class LoadBalancerTest : public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::LoadBalancerTest );
			  CPPUNIT_TEST( testLoadBalancer );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  LoadBalancerTest();
			  ~LoadBalancerTest();
			  void setUp();
			  void tearDown();

			protected:
			  void testLoadBalancer();
			private:
			  SDPA_DECLARE_LOGGER();
			};
		}
}
#endif
