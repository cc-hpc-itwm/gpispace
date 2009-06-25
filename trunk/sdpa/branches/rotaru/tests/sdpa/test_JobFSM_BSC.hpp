#ifndef JOBFSMBSCTEST_HPP_
#define JOBFSMBSCTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/logging.hpp"
#include "sdpa/jobFSM/BSC/JobFSM.hpp"

namespace sdpa {
		namespace tests {
			class JobFSMTest_BSC : public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE(sdpa::tests::JobFSMTest_BSC);
			  CPPUNIT_TEST(testJobFSM_BSC);
			  CPPUNIT_TEST_SUITE_END();

			public:
			  JobFSMTest_BSC();
			  ~JobFSMTest_BSC();
			  void setUp();
			  void tearDown();

			protected:
			  void testJobFSM_BSC();
			private:
			  SDPA_DECLARE_LOGGER();
			  JobFSM m_JobFSM;
			};
		}
}
#endif
