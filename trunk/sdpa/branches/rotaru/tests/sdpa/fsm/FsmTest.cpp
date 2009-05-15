#include "FsmTest.hpp"

using namespace sdpa::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( CFsmTest );

CFsmTest::CFsmTest()
    : SDPA_INIT_LOGGER("sdpa.tests.fsmTest")
{}

CFsmTest::~CFsmTest()
{}

void CFsmTest::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");
}

void CFsmTest::tearDown() { //stop the finite state machine
	SDPA_LOG_DEBUG("tearDown");
}

void CFsmTest::testFSM() {

}
