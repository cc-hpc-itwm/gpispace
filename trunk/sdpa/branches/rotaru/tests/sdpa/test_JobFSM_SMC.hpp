#ifndef JOBFSMTESTSMC_HPP_
#define JOBFSMTESTSMC_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/jobFSM/SMC/JobFSM.hpp"

namespace sdpa {
		namespace tests {
			class JobFSMTest_SMC : public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::JobFSMTest_SMC );
			  CPPUNIT_TEST( testJobFSM_SMC );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  JobFSMTest_SMC();
			  ~JobFSMTest_SMC();
			  void setUp();
			  void tearDown();

			protected:
			  void testJobFSM_SMC();
			private:
			  SDPA_DECLARE_LOGGER();
			  sdpa::fsm::JobFSM m_JobFSM;
			};
		}
}
#endif 
