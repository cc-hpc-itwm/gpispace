/*
 * =====================================================================================
 *
 *       Filename:  test_SerializeSharedPtr.hpp
 *
 *    Description:  test all components, each with a real gwes, using a real user client
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
#ifndef TEST_SERIALIZE_JOB_HPP_
#define TEST_SERIALIZE_JOB_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"

namespace sdpa {
	namespace tests {
		class TestSerializeSharedPtr: public CPPUNIT_NS::TestFixture {
		  CPPUNIT_TEST_SUITE( sdpa::tests::TestSerializeSharedPtr );

		  CPPUNIT_TEST( testSerializeBoostShPtrToTxt );
		  CPPUNIT_TEST( testSerializeBoostShPtrToXml );
		  CPPUNIT_TEST( testSerializeBoostShPtrDerived );
		  CPPUNIT_TEST( testSerializeNormalPtr );
		  CPPUNIT_TEST( testSerializeMapPtr );
		  CPPUNIT_TEST_SUITE_END();

		public:
		  TestSerializeSharedPtr();
		  ~TestSerializeSharedPtr();
		  void setUp();
		  void tearDown();

		protected:
		  void testSerializeBoostShPtrToTxt();
		  void testSerializeBoostShPtrToXml();
		  void testSerializeBoostShPtrDerived();
		  void testSerializeNormalPtr();
		  void testSerializeMapPtr();

		private:
		  SDPA_DECLARE_LOGGER();
		};
	}
}
#endif
