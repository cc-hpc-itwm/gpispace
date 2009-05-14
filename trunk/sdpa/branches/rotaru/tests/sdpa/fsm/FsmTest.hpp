#ifndef FSMTEST_HPP_
#define FSMTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include <sdpa/memory.hpp>
#include <sdpa/logging.hpp>

namespace sdpa {
		namespace tests {
			class CFsmTest : public CppUnit::TestFixture {
			  CPPUNIT_TEST_SUITE( sdpa::tests::CFsmTest );
			  CPPUNIT_TEST( testFSM );
			  CPPUNIT_TEST_SUITE_END();

			public:
			  CFsmTest();
			  void setUp();
			  void tearDown();

			protected:
			  void testFSM();
			private:
			  SDPA_DECLARE_LOGGER();
			};
		}
}
#endif /* FSMTEST_HPP_ */
