/*
 * =====================================================================================
 *
 *       Filename:  test_udp.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  10/25/2009 04:24:41 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <unistd.h>

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>

#include <seda/StageFactory.hpp>
#include <seda/EventCountStrategy.hpp>
#include <seda/AccumulateStrategy.hpp>
#include <seda/DiscardStrategy.hpp>

#include <seda/comm/comm.hpp>
#include <seda/comm/Connection.hpp>
#include <seda/comm/UDPConnection.hpp>
#include <seda/comm/ConnectionFactory.hpp>
#include <seda/comm/ConnectionStrategy.hpp>

int main(int argc, char **argv)
{
  fhg::log::Configurator::configure();
  seda::comm::initialize(argc, argv);

  int errcount(0);

  {
    // allocate factories
    seda::comm::ConnectionFactory::ptr_t cFactory(new seda::comm::ConnectionFactory());
    seda::StageFactory::Ptr sFactory(new seda::StageFactory());

    seda::EventCountStrategy *p1_ecs;
    seda::EventCountStrategy *p2_ecs;

    seda::AccumulateStrategy *p1_acc;
    seda::AccumulateStrategy *p2_acc;


    seda::comm::Connection::ptr_t conn_1;

    // create the first "process"
    {
      LOG(INFO, "creating process-1 stages...");

      seda::comm::ConnectionParameters params("udp", "127.0.0.1:5000", "process-1");
      conn_1 = cFactory->createConnection(params);

      // process-1 has two stages, a counting/discarding and the network
      seda::Strategy::Ptr net(new seda::comm::ConnectionStrategy("p1-final", conn_1));
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
      LOG(INFO, "creating process-2 stages...");

      seda::comm::ConnectionParameters params("udp", "127.0.0.1:5001", "process-2");
      seda::comm::Connection::ptr_t conn_2 = cFactory->createConnection(params);

      conn_1->locator()->insert("process-2", "127.0.0.1:5001");
      conn_2->locator()->insert("process-1", "127.0.0.1:5000");

      usleep (500);

      // process-2 has two stages, a counting/discarding and the network
      seda::Strategy::Ptr net(new seda::comm::ConnectionStrategy("p2-final", conn_2));
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
        seda::comm::SedaMessage::Ptr(new seda::comm::SedaMessage("process-1", "process-2", "hello process 2", 1))
    );

    // send an event to the p2-network stage
    seda::StageRegistry::instance().lookup("p2-net")->send(
        seda::comm::SedaMessage::Ptr(new seda::comm::SedaMessage("process-2", "process-1", "hello process 1", 2))
    );

    // wait for both messages to be received
    p1_ecs->wait(1, 1000);
    p2_ecs->wait(1, 1000);

    // inspect the received messages
    seda::comm::SedaMessage *p1_msg(dynamic_cast<seda::comm::SedaMessage*>(p1_acc->begin()->get()));
    if (p1_msg == NULL)
    {
      LOG(ERROR, "send from p2 to p1 failed, no message has been received");
      ++errcount;
    }
    else
    {
      if (p1_msg->payload() != "hello process 1")
      {
        LOG(ERROR, "received message's payload does not match");
        LOG(ERROR, "got: " << p1_msg->payload() << " expected: " << "hello process 1");
        ++errcount;
      }
    }

    seda::comm::SedaMessage *p2_msg(dynamic_cast<seda::comm::SedaMessage*>(p2_acc->begin()->get()));
    if (p2_msg == NULL)
    {
      LOG(ERROR, "send from p1 to p2 failed, no message has been received");
      ++errcount;
    }
    else
    {
      if (p2_msg->payload() != "hello process 2")
      {
        LOG(ERROR, "received message's payload does not match");
        LOG(ERROR, "got: " << p2_msg->payload() << " expected: " << "hello process 2");
        ++errcount;
      }
    }

    // shut everything down
    seda::StageRegistry::instance().stopAll();
    seda::StageRegistry::instance().clear();
  }

  LOG(INFO, "done.");

  seda::comm::shutdown();
  return errcount;
}
