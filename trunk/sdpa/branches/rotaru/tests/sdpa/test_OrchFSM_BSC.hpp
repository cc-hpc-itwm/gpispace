#ifndef FSMTEST_HPP_
#define FSMTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/orchFSM/BSC/OrchFSM.hpp"

namespace sdpa {
		namespace tests {
			class OrchFSMTest_BSC : public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::OrchFSMTest_BSC );
			  CPPUNIT_TEST( testOrchFSM_BSC );
			  CPPUNIT_TEST_SUITE_END();

			public:
				OrchFSMTest_BSC();
			  ~OrchFSMTest_BSC();
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
