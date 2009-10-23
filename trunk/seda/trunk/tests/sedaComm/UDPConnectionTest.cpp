#include "UDPConnectionTest.hpp"

#include <iostream>
#include <string>
#include <stdexcept>
#include <csignal>
#include <stdint.h>
#include <sys/wait.h>

#include <seda/seda-config.hpp>
#include <seda/Stage.hpp>
#include <seda/StageFactory.hpp>
#include <seda/AccumulateStrategy.hpp>
#include <seda/EventCountStrategy.hpp>
#include <seda/DiscardStrategy.hpp>
#include <seda/comm/ConnectionStrategy.hpp>
#include <seda/comm/SedaMessage.hpp>
#include <seda/comm/UDPConnection.hpp>
#include <seda/comm/ConnectionFactory.hpp>

using namespace seda::comm::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( UDPConnectionTest );

UDPConnectionTest::UDPConnectionTest()
{
}

UDPConnectionTest::~UDPConnectionTest()
{
}

void
UDPConnectionTest::setUp() {
}

void
UDPConnectionTest::tearDown() {
}

//void
//UDPConnectionTest::testConnectionFactory() {
//  seda::comm::ConnectionParameters params("zmq", "localhost", "test");
//  params.put("iface_in", "*:5222");
//  params.put("iface_out", "*");
//  try {
//    seda::comm::ConnectionFactory::ptr_t factory(new seda::comm::ConnectionFactory());
//    seda::comm::Connection::ptr_t conn = factory->createConnection(params);
//    conn->start();
//    seda::comm::SedaMessage msg1("test", "test", "foo");
//    conn->send(msg1);
//
//    seda::comm::SedaMessage msg2;
//    conn->recv(msg2);
//    CPPUNIT_ASSERT_MESSAGE("received payload differs from sent payload", msg1.payload() == msg2.payload());
//    conn->stop();
//  }
//  catch (const std::exception &ex) {
//    SEDA_LOG_ERROR("could not create the connection: " << ex.what());
//    CPPUNIT_ASSERT_MESSAGE("connection could not be created!", false);
//  }
//  catch (...)
//  {
//    CPPUNIT_ASSERT_MESSAGE("connection could not be created: unknown reason!", false);
//  }
//}
//
//void
//ZMQConnectionTest::testConnectionStrategy() {
//  try {
//    // allocate factories
//    seda::comm::ConnectionFactory::ptr_t cFactory(new seda::comm::ConnectionFactory());
//    seda::StageFactory::Ptr sFactory(new seda::StageFactory());
//
//    seda::EventCountStrategy *p1_ecs;
//    seda::EventCountStrategy *p2_ecs;
//
//    seda::AccumulateStrategy *p1_acc;
//    seda::AccumulateStrategy *p2_acc;
//    // create the first "process"
//    {
//      seda::comm::ConnectionParameters params("zmq", "localhost", "process-1");
//      params.put("iface_in", "*:5222");
//      params.put("iface_out", "*");
//      seda::comm::Connection::ptr_t conn = cFactory->createConnection(params);
//
//      // process-1 has two stages, a counting/discarding and the network
//      seda::Strategy::Ptr net(new seda::comm::ConnectionStrategy("p1-final", conn));
//      seda::Stage::Ptr net_stage(sFactory->createStage("p1-net", net));
//
//      seda::Strategy::Ptr discard(new seda::DiscardStrategy());
//      p1_ecs = new seda::EventCountStrategy(discard);
//      discard = seda::Strategy::Ptr(p1_ecs);
//      p1_acc = new seda::AccumulateStrategy(discard);
//      discard = seda::Strategy::Ptr(p1_acc);
//      seda::Stage::Ptr final(sFactory->createStage("p1-final", discard));
//    }
//
//    // process-2
//    {
//      seda::comm::ConnectionParameters params("zmq", "localhost", "process-2");
//      params.put("iface_in", "*:5223");
//      params.put("iface_out", "*");
//      seda::comm::Connection::ptr_t conn = cFactory->createConnection(params);
//
//      // process-2 has two stages, a counting/discarding and the network
//      seda::Strategy::Ptr net(new seda::comm::ConnectionStrategy("p2-final", conn));
//      seda::Stage::Ptr net_stage(sFactory->createStage("p2-net", net));
//
//      seda::Strategy::Ptr discard(new seda::DiscardStrategy());
//      p2_ecs = new seda::EventCountStrategy(discard);
//      discard = seda::Strategy::Ptr(p2_ecs);
//      p2_acc = new seda::AccumulateStrategy(discard);
//      discard = seda::Strategy::Ptr(p2_acc);
//      seda::Stage::Ptr final(sFactory->createStage("p2-final", discard));
//    }
//    seda::StageRegistry::instance().startAll();
//
//    // send an event to the p1-network stage
//    seda::StageRegistry::instance().lookup("p1-net")->send(
//        seda::comm::SedaMessage::Ptr(new seda::comm::SedaMessage("process-1", "process-2", "hello 2"))
//    );
//
//    // send an event to the p2-network stage
//    seda::StageRegistry::instance().lookup("p2-net")->send(
//        seda::comm::SedaMessage::Ptr(new seda::comm::SedaMessage("process-2", "process-1", "hello 1"))
//    );
//
//    // wait for both messages to be received
//    p1_ecs->wait(1, 1000);
//    p2_ecs->wait(1, 1000);
//
//    CPPUNIT_ASSERT_EQUAL(std::size_t(1), p1_ecs->count());
//    CPPUNIT_ASSERT_EQUAL(std::size_t(1), p1_ecs->count());
//
//    // inspect the received messages
//    seda::comm::SedaMessage *p1_msg(dynamic_cast<seda::comm::SedaMessage*>(p1_acc->begin()->get()));
//    seda::comm::SedaMessage *p2_msg(dynamic_cast<seda::comm::SedaMessage*>(p2_acc->begin()->get()));
//
//    CPPUNIT_ASSERT(p1_msg != NULL);
//    CPPUNIT_ASSERT(p2_msg != NULL);
//
//    CPPUNIT_ASSERT_EQUAL(std::string("hello 1"), p1_msg->payload());
//    CPPUNIT_ASSERT_EQUAL(std::string("hello 2"), p2_msg->payload());
//
//    // shut everything down
//    seda::StageRegistry::instance().stopAll();
//    seda::StageRegistry::instance().clear();
//  }
//  catch (const std::exception &ex) {
//    SEDA_LOG_ERROR("could not create the connection: " << ex.what());
//    CPPUNIT_ASSERT_MESSAGE("connection could not be created!", false);
//  }
//  catch (...)
//  {
//    CPPUNIT_ASSERT_MESSAGE("connection could not be created: unknown reason!", false);
//  }
//}

void
UDPConnectionTest::testSendReceive() {
  seda::comm::Locator::ptr_t locator(new seda::comm::Locator());
  seda::comm::UDPConnection conn(locator, "test", "127.0.0.1", 5222);
  try {
    conn.start();

    for (std::size_t cnt(0); cnt < 10000; ++cnt)
    {
      seda::comm::SedaMessage msg1("test", "test", "foo");
      conn.send(msg1);
      seda::comm::SedaMessage msg2;
      conn.recv(msg2);
      CPPUNIT_ASSERT_MESSAGE("received payload differs from sent payload", msg1.payload() == msg2.payload());
    }

    conn.stop();
  } catch (const std::exception &ex) {
    LOG(ERROR, "could not start/stop the udp-connection: " << ex.what());
    CPPUNIT_ASSERT_MESSAGE("udp connection could not be started", false);
  } catch(...) {
    LOG(ERROR, "could not start/stop the zmq-connection (unknown)");
    CPPUNIT_ASSERT_MESSAGE("udp connection could not be started", false);
  }
}

void
UDPConnectionTest::testStartStop() {
  seda::comm::Locator::ptr_t locator(new seda::comm::Locator());
  seda::comm::UDPConnection conn(locator, "test", "127.0.0.1", 5222);
  try {
    for (std::size_t i(0); i < 3; i++) {
      conn.start();

      seda::comm::SedaMessage msg1("test", "test", "foo");
      conn.send(msg1);
      seda::comm::SedaMessage msg2;
      conn.recv(msg2);
      CPPUNIT_ASSERT_MESSAGE("received payload differs from sent payload", msg1.payload() == msg2.payload());

      conn.stop();
      sleep(1);
    }
  } catch (const std::exception &ex) {
    LOG(ERROR, "could not start/stop the udp-connection: " << ex.what());
    CPPUNIT_ASSERT_MESSAGE("udp connection could not be started", false);
  } catch(...) {
    LOG(ERROR, "could not start/stop the zmq-connection (unknown)");
    CPPUNIT_ASSERT_MESSAGE("udp connection could not be started", false);
  }
}
