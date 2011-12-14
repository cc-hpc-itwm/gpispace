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

#include <fhglog/fhglog.hpp>

#include <seda/StageFactory.hpp>
#include <seda/EventCountStrategy.hpp>
#include <seda/AccumulateStrategy.hpp>
#include <seda/DiscardStrategy.hpp>
#include <seda/LossyDaemonStrategy.hpp>

#include <seda/comm/comm.hpp>
#include <seda/comm/Connection.hpp>
#include <seda/comm/UDPConnection.hpp>
#include <seda/comm/ConnectionFactory.hpp>
#include <seda/comm/ConnectionStrategy.hpp>

#include <seda/comm/delivery_service.hpp>
#include <seda/comm/ServiceThread.hpp>

struct handler
{
  handler()
    : called (false)
  {}

  bool called;

  void deliveryFailed(seda::comm::SedaMessage::Ptr)
  {
    std::clog << "transmission of message failed!" << std::endl;
    called = true;
  }
};

int main(int argc, char **argv)
{
  fhg::log::Configurator::configure();
  seda::comm::initialize(argc, argv);

  int errcount(0);
  {
    typedef seda::comm::delivery_service<seda::comm::SedaMessage::Ptr, seda::comm::SedaMessage::message_id_type, seda::Stage> seda_msg_delivery_service;

    seda::StageFactory sFactory;
    seda::Strategy::Ptr discard(new seda::DiscardStrategy("discard"));
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    seda::LossyDaemonStrategy::Ptr lossy(new seda::LossyDaemonStrategy(ecs, 0));
    seda::Stage::Ptr stage(sFactory.createStage("final", lossy));

    seda::comm::ServiceThread service_thread;
    seda_msg_delivery_service service(service_thread.io_service(), 500);

    handler h;
    seda_msg_delivery_service::callback_handler f = std::bind1st(std::mem_fun(&handler::deliveryFailed), &h);
    service.register_callback_handler(f);

    stage->start();
    service.start();
    service_thread.start();

    // test successful send
    {
      std::clog << "testing successful send of a single message...";
      seda::comm::SedaMessage::Ptr m1(new seda::comm::SedaMessage("from", "to", "hello-1", 42));
      service.send(stage.get(), m1, m1->id(), 1, 1);

      if (ecs->wait(1, 1000))
      {
        service.acknowledge(m1->id());
        std::clog << "ok" << std::endl;
      }
      else
      {
        std::clog << "failed" << std::endl;
        ++errcount;
      }
    }
    ecs->reset();

    // test unsuccessful send
    {
      lossy->set_probability (1);

      std::clog << "testing unsuccessful send of a single message...";
      seda::comm::SedaMessage::Ptr m1(new seda::comm::SedaMessage("from", "to", "hello-1", 42));
      service.send(stage.get(), m1, m1->id(), 1, 1);

      if (ecs->wait(1, 4000))
      {
        std::clog << "failed! message delivered" << std::endl;
        ++errcount;
      }
      else
      {
        if (h.called)
        {
          std::clog << "ok" << std::endl;
          h.called = false;
        }
        else
        {
          std::clog << "failed! callback handler has not been called!" << std::endl;
          ++errcount;
        }
      }
    }
    ecs->reset();

    // test resend sucess
    {
      lossy->set_probability (.5);

      std::clog << "testing successful send of a single message after retry...";
      seda::comm::SedaMessage::Ptr m1(new seda::comm::SedaMessage("from", "to", "hello-1", 42));
      service.send(stage.get(), m1, m1->id(), 1, 10);

      if (ecs->wait(2, 5000))
      {
        service.acknowledge(m1->id());
        std::clog << "ok" << std::endl;
      }
      else
      {
        std::clog << "failed" << std::endl;
        ++errcount;
      }
    }
    ecs->reset();

    // test resend and cancel
    {
      lossy->set_probability (0);

      std::clog << "testing cancel after resend...";
      seda::comm::SedaMessage::Ptr m1(new seda::comm::SedaMessage("from", "to", "hello-1", 42));
      service.send(stage.get(), m1, m1->id(), 1, 5);

      if (ecs->wait(2, 2000))
      {
        service.cancel(m1->id());
      }
      ecs->wait(3, 1000);

      if (! ecs->wait(4, 100))
      {
        std::clog << "ok" << std::endl;
      }
      else
      {
        std::clog << "failed" << std::endl;
        ++errcount;
      }
    }
    ecs->reset();

    // restarting service and thread
    {
      service.stop();
      service_thread.stop();

      service.start();
      service_thread.start();

      std::clog << "testing send of a single message with retry after restart...";
      seda::comm::SedaMessage::Ptr m1(new seda::comm::SedaMessage("from", "to", "hello-1", 42));
      service.send(stage.get(), m1, m1->id(), 1, 1);

      if (ecs->wait(2, 3000))
      {
        service.acknowledge(m1->id());
        std::clog << "ok" << std::endl;
      }
      else
      {
        std::clog << "failed" << std::endl;
        ++errcount;
      }
    }
    ecs->reset();

    stage->stop();
    service_thread.stop();
    service.stop();
  }

  // shut everything down
  seda::StageRegistry::instance().stopAll();
  seda::StageRegistry::instance().clear();

  LOG(INFO, "done.");

  seda::comm::shutdown();
  return errcount;
}
