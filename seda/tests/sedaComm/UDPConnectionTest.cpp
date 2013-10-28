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

void
UDPConnectionTest::setUp() {
}

void
UDPConnectionTest::tearDown() {
  // shut everything down
  seda::StageRegistry::instance().stopAll();
  seda::StageRegistry::instance().clear();
}

class B {
  public:
    typedef seda::shared_ptr<B> ptr_t;

    B() {
      foo = new int();
    }

    ~B()
    {
      delete foo;
      foo = NULL;
    }
  private:
    int *foo;
};

class A {
  public:
    typedef seda::shared_ptr<A> ptr_t;

    A(const std::string &a_name, const B::ptr_t &a_b)
      : name_(a_name)
      , b_(a_b)
    {}
  private:
    std::string name_;
    B::ptr_t b_;
};

void UDPConnectionTest::testSharedPtr() {
  B::ptr_t b(new B());
  {
    A::ptr_t a(new A("a", b));
  }
}

void UDPConnectionTest::testConnectionFactory() {
  seda::comm::ConnectionParameters params("udp", "127.0.0.1", "test");
  try {
    seda::comm::ConnectionFactory::ptr_t factory(new seda::comm::ConnectionFactory());
    seda::comm::Connection::ptr_t conn = factory->createConnection(params);
    conn->start();
    seda::comm::SedaMessage msg1("test", "test", "foo", 4711);
    conn->send(msg1);

    seda::comm::SedaMessage msg2;
    conn->recv(msg2);
    CPPUNIT_ASSERT_MESSAGE("received payload differs from sent payload", msg1.payload() == msg2.payload());
    CPPUNIT_ASSERT_MESSAGE("received message id differs from sent id", msg1.id() == msg2.id());
    CPPUNIT_ASSERT_MESSAGE("message id is strange", msg2.id() == 4711);
    conn->stop();
  }
  catch (const std::exception &ex) {
    LOG(ERROR, "could not create the connection: " << ex.what());
    CPPUNIT_ASSERT_MESSAGE("connection could not be created!", false);
  }
  catch (...)
  {
    CPPUNIT_ASSERT_MESSAGE("connection could not be created: unknown reason!", false);
  }
}

void UDPConnectionTest::testConnectionSharedPointer()
{
  seda::comm::ConnectionFactory::ptr_t cFactory(new seda::comm::ConnectionFactory());
  seda::comm::ConnectionParameters params("udp", "127.0.0.1", "process-1");
  for (std::size_t i(0); i < 1000; ++i)
  {
    seda::comm::Connection::ptr_t conn = cFactory->createConnection(params);
    conn->start();
    conn->stop();
  }
}

void UDPConnectionTest::testConnectionStrategy() {
  try {
    // allocate factories
    seda::comm::ConnectionFactory::ptr_t cFactory(new seda::comm::ConnectionFactory());
    seda::StageFactory::Ptr sFactory(new seda::StageFactory());

    seda::EventCountStrategy *p1_ecs;
    seda::EventCountStrategy *p2_ecs;

    seda::AccumulateStrategy *p1_acc;
    seda::AccumulateStrategy *p2_acc;
    // create the first "process"
    {
      seda::comm::ConnectionParameters params("udp", "127.0.0.1:5000", "process-1");
      seda::comm::Connection::ptr_t conn = cFactory->createConnection(params);
      conn->locator()->insert("process-2", "127.0.0.1:5001");

      // process-1 has two stages, a counting/discarding and the network
      seda::Strategy::Ptr net(new seda::comm::ConnectionStrategy("p1-final", conn));
      seda::Stage::Ptr net_stage(sFactory->createStage("p1-net", net));

      seda::Strategy::Ptr discard(new seda::DiscardStrategy());
      p1_ecs = new seda::EventCountStrategy(discard);
      discard = seda::Strategy::Ptr(p1_ecs);
      p1_acc = new seda::AccumulateStrategy(discard);
      discard = seda::Strategy::Ptr(p1_acc);
      seda::Stage::Ptr final(sFactory->createStage("p1-final", discard));
    }

    // process-2
    {
      seda::comm::ConnectionParameters params("udp", "127.0.0.1:5001", "process-2");
      seda::comm::Connection::ptr_t conn = cFactory->createConnection(params);
      conn->locator()->insert("process-1", "127.0.0.1:5000");

      // process-2 has two stages, a counting/discarding and the network
      seda::Strategy::Ptr net(new seda::comm::ConnectionStrategy("p2-final", conn));
      seda::Stage::Ptr net_stage(sFactory->createStage("p2-net", net));

      seda::Strategy::Ptr discard(new seda::DiscardStrategy());
      p2_ecs = new seda::EventCountStrategy(discard);
      discard = seda::Strategy::Ptr(p2_ecs);
      p2_acc = new seda::AccumulateStrategy(discard);
      discard = seda::Strategy::Ptr(p2_acc);
      seda::Stage::Ptr final(sFactory->createStage("p2-final", discard));
    }
    seda::StageRegistry::instance().startAll();

    // send an event to the p1-network stage
    seda::StageRegistry::instance().lookup("p1-net")->send(
        seda::comm::SedaMessage::Ptr(new seda::comm::SedaMessage("process-1", "process-2", "hello 2", 1))
    );

    // send an event to the p2-network stage
    seda::StageRegistry::instance().lookup("p2-net")->send(
        seda::comm::SedaMessage::Ptr(new seda::comm::SedaMessage("process-2", "process-1", "hello 1", 2))
    );

    // wait for both messages to be received
    p1_ecs->wait(1, 1000);
    p2_ecs->wait(1, 1000);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), p1_ecs->count());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), p1_ecs->count());

    // inspect the received messages
    seda::comm::SedaMessage *p1_msg(dynamic_cast<seda::comm::SedaMessage*>(p1_acc->begin()->get()));
    seda::comm::SedaMessage *p2_msg(dynamic_cast<seda::comm::SedaMessage*>(p2_acc->begin()->get()));

    CPPUNIT_ASSERT(p1_msg != NULL);
    CPPUNIT_ASSERT(p2_msg != NULL);

	CPPUNIT_ASSERT(p1_msg->id() == 2);
	CPPUNIT_ASSERT(p2_msg->id() == 1);

    CPPUNIT_ASSERT_EQUAL(std::string("hello 1"), p1_msg->payload());
    CPPUNIT_ASSERT_EQUAL(std::string("hello 2"), p2_msg->payload());

    // shut everything down
    seda::StageRegistry::instance().stopAll();
    seda::StageRegistry::instance().clear();
  }
  catch (const std::exception &ex) {
    LOG(ERROR, "could not create the connection: " << ex.what());
    CPPUNIT_ASSERT_MESSAGE("connection could not be created!", false);
  }
  catch (...)
  {
    CPPUNIT_ASSERT_MESSAGE("connection could not be created: unknown reason!", false);
  }
}

void
UDPConnectionTest::testSendReceive() {
  seda::comm::Locator::ptr_t locator(new seda::comm::Locator());
  seda::comm::UDPConnection conn(locator, "test", "127.0.0.1", 5222);
  try {
    conn.start();

    for (std::size_t cnt(0); cnt < 1000; ++cnt)
    {
      seda::comm::SedaMessage msg1("test", "test", "foo", cnt);
      conn.send(msg1);
      seda::comm::SedaMessage msg2;
      conn.recv(msg2);
      CPPUNIT_ASSERT_MESSAGE("received payload differs from sent payload", msg1.payload() == msg2.payload());
      CPPUNIT_ASSERT_MESSAGE("received msg id differs from sent one", msg1.id() == msg2.id());
    }

    conn.stop();
  } catch (const std::exception &ex) {
    LOG(ERROR, "could not start/stop the udp-connection: " << ex.what());
    CPPUNIT_ASSERT_MESSAGE("udp connection could not be started", false);
  } catch(...) {
    LOG(ERROR, "could not start/stop the udp-connection (unknown)");
    CPPUNIT_ASSERT_MESSAGE("udp connection could not be started", false);
  }
}

void
UDPConnectionTest::testSendReceiveNetwork() {
  LOG(INFO, "testing send receive in a circle");
  const std::size_t num_messages (1000);

  seda::comm::Locator::ptr_t locator(new seda::comm::Locator());
  locator->insert("test1", "127.0.0.1:0");
  locator->insert("test2", "127.0.0.1:0");
  locator->insert("test3", "127.0.0.1:0");

  seda::comm::UDPConnection conn1(locator, "test1");
  seda::comm::UDPConnection conn2(locator, "test2");
  seda::comm::UDPConnection conn3(locator, "test3");

  conn1.start(); conn2.start(); conn3.start();
  {
    for (std::size_t cnt(0); cnt < num_messages; ++cnt)
    {
      seda::comm::SedaMessage msg12("test1", "test2", "test-1-2", 0*num_messages + cnt);
      conn1.send(msg12);
      {
        seda::comm::SedaMessage msg;
        conn2.recv(msg);
        CPPUNIT_ASSERT_MESSAGE("received payload differs from sent payload", msg12.payload() == msg.payload());
      }

      seda::comm::SedaMessage msg23("test2", "test3", "test-2-3", 1*num_messages + cnt);
      conn2.send(msg23);
      {
        seda::comm::SedaMessage msg;
        conn3.recv(msg);
        CPPUNIT_ASSERT_MESSAGE("received payload differs from sent payload", msg23.payload() == msg.payload());
      }

      seda::comm::SedaMessage msg31("test3", "test1", "test-3-1", 2*num_messages + cnt);
      conn3.send(msg31);
      {
        seda::comm::SedaMessage msg;
        conn1.recv(msg);
        CPPUNIT_ASSERT_MESSAGE("received payload differs from sent payload", msg31.payload() == msg.payload());
      }
    }
  }

  conn1.stop(); conn2.stop(); conn3.stop();
}

void
UDPConnectionTest::testStartStop() {
  seda::comm::Locator::ptr_t locator(new seda::comm::Locator());
  seda::comm::UDPConnection conn(locator, "test", "127.0.0.1", 5222);
  try {
    for (std::size_t i(0); i < 3; i++) {
      conn.start();

      seda::comm::SedaMessage msg1("test", "test", "foo", 42);
      conn.send(msg1);
      seda::comm::SedaMessage msg2;
      conn.recv(msg2);
      CPPUNIT_ASSERT_MESSAGE("received payload differs from sent payload", msg1.payload() == msg2.payload());
	  CPPUNIT_ASSERT( msg2.id() == msg1.id() );

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
