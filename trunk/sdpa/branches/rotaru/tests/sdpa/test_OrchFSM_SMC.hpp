#ifndef ORCHFSMTESTSMC_HPP_
#define ORCHFSMTESTSMC_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/orchFSM/SMC/OrchFSM.hpp"

namespace sdpa {
		namespace tests {
			class OrchFSMTest_SMC : public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::OrchFSMTest_SMC );
			  CPPUNIT_TEST( testOrchFSM_SMC );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  OrchFSMTest_SMC();
			  ~OrchFSMTest_SMC();
			  void setUp();
			  void tearDown();

			protected:
			  void testOrchFSM_SMC();
			private:
			  SDPA_DECLARE_LOGGER();
			  sdpa::fsm::OrchFSM m_OrchFSM;
			};
		}
}
#endif
