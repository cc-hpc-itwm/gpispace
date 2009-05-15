#ifndef FSMTEST_HPP_
#define FSMTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/fsm/JobFSM.hpp"

namespace sdpa {
		namespace tests {
			class CFsmTest : public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::CFsmTest );
			  CPPUNIT_TEST( testFSM );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  CFsmTest();
			  ~CFsmTest();
			  void setUp();
			  void tearDown();

			protected:
			  void testFSM();
			private:
			  SDPA_DECLARE_LOGGER();
			  sdpa::fsm::JobFSM m_JobFSM;
			};
		}
}
#endif /* FSMTEST_HPP_ */
