/*
 * =====================================================================================
 *
 *       Filename:  test_Components.hpp
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
#ifndef SERIALIZE_SHPTR_TEST_HPP_
#define SERIALIZE_SHPTR_TEST_HPP_

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"

namespace sdpa {
	namespace tests {
		class TestSerializeJobPtr: public CPPUNIT_NS::TestFixture {
		  CPPUNIT_TEST_SUITE( sdpa::tests::TestSerializeJobPtr );
		  CPPUNIT_TEST( testSerializeJobPtr );
		  CPPUNIT_TEST( testSerializeSdpaJobPtr );
		  CPPUNIT_TEST( testSerializeMapJobPtr );
		  CPPUNIT_TEST( testSerializeSdpaJobSharedPtr );
		  CPPUNIT_TEST( testSerializeMapSdpaJobSharedPtr );
		  CPPUNIT_TEST( testSerializeJobManager );
		  CPPUNIT_TEST_SUITE_END();

		public:
		  TestSerializeJobPtr();
		  ~TestSerializeJobPtr();
		  void setUp();
		  void tearDown();

		protected:
		  void testSerializeJobPtr();
		  void testSerializeSdpaJobPtr();
		  void testSerializeMapJobPtr();
		  void testSerializeSdpaJobSharedPtr();
		  void testSerializeMapSdpaJobSharedPtr();
		  void testSerializeJobManager();

		private:
		  SDPA_DECLARE_LOGGER();
		};
	}
}
#endif
