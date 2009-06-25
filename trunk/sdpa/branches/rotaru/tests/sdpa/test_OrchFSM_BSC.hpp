#ifndef FSMTEST_HPP_
#define FSMTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/orchFSM/BSC/OrchFSM.hpp"

namespace sdpa {
		namespace tests {
			class COrchFSMTest : public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::COrchFSMTest );
			  CPPUNIT_TEST( testOrchFSM );
			  CPPUNIT_TEST_SUITE_END();

			public:
				COrchFSMTest();
			  ~COrchFSMTest();
			  void setUp();
			  void tearDown();

			protected:
			  void testOrchFSM();
			private:
			  SDPA_DECLARE_LOGGER();
			  OrchFSM m_OrchFSM;
			};
		}
}
#endif /* FSMTEST_HPP_ */
