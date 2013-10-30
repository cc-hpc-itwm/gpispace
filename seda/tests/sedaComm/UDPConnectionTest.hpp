#ifndef SEDA_TESTS_COMM_ZMQCONN_HPP
#define SEDA_TESTS_COMM_ZMQCONN_HPP 1

#include <cppunit/extensions/HelperMacros.h>
#include <seda/common.hpp>
#include <stdint.h>
#include <unistd.h>

namespace seda { namespace comm { namespace tests {
  class UDPConnectionTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE( seda::comm::tests::UDPConnectionTest );
    CPPUNIT_TEST( testConnectionFactory );
    CPPUNIT_TEST( testStartStop );
    CPPUNIT_TEST( testSendReceive );
    CPPUNIT_TEST( testSendReceiveNetwork );
    CPPUNIT_TEST( testConnectionStrategy );
    CPPUNIT_TEST( testSharedPtr );
    CPPUNIT_TEST( testConnectionSharedPointer );
    CPPUNIT_TEST_SUITE_END();

    private:
    public:
    void setUp();
    void tearDown();

    protected:
    void testConnectionFactory();
    void testSendReceive();
    void testSendReceiveNetwork();
    void testStartStop();
    void testConnectionStrategy();
    void testConnectionStrategyMinimal();
    void testConnectionSharedPointer();
    void testSharedPtr();
  };
}}}

#endif
