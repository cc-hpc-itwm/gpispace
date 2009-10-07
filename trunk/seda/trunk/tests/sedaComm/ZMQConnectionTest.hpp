#ifndef SEDA_TESTS_COMM_ZMQCONN_HPP
#define SEDA_TESTS_COMM_ZMQCONN_HPP 1

#include <cppunit/extensions/HelperMacros.h>
#include <seda/logging.hpp>
#include <stdint.h>
#include <unistd.h>

namespace seda { namespace comm { namespace tests {
  class ZMQConnectionTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE( seda::comm::tests::ZMQConnectionTest );
    CPPUNIT_TEST( testAbortException );
    CPPUNIT_TEST( testSendReceive );
    CPPUNIT_TEST( testStartStop );
    CPPUNIT_TEST( testConnectionFactory );
    CPPUNIT_TEST( testConnectionStrategy );
    CPPUNIT_TEST_SUITE_END();

    private:
    public:
    ZMQConnectionTest();
    ~ZMQConnectionTest();
    void setUp();
    void tearDown();

    protected:

    SEDA_DECLARE_LOGGER();
    pid_t zmq_server_pid_;
    uint32_t zmq_server_port_;

    bool start_zmq_server(pid_t *pid, uint32_t *port);
    bool stop_zmq_server(pid_t *pid);


    void testSendReceive();
    void testStartStop();
    void testAbortException();
    void testConnectionFactory();
    void testConnectionStrategy();
  };
}}}

#endif
