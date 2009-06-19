#ifndef FSMTEST_HPP_
#define FSMTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/jobFSM/JobFSM.hpp"

namespace sdpa {
		namespace tests {
			class CJobFSMTest : public CPPUNIT_NS::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::CJobFSMTest );
			  CPPUNIT_TEST( testJobFSM );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  CJobFSMTest();
			  ~CJobFSMTest();
			  void setUp();
			  void tearDown();

			protected:
			  void testJobFSM();
			private:
			  SDPA_DECLARE_LOGGER();
			  JobFSM m_JobFSM;
			};
		}
}
#endif /* FSMTEST_HPP_ */
