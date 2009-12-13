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
#include <seda/DiscardStrategy.hpp>

#include <seda/comm/comm.hpp>
#include <seda/comm/delivery_service.hpp>
#include <seda/comm/ServiceThread.hpp>

#include <sdpa/events/SubmitJobEvent.hpp>

struct handler
{
  void operator()(sdpa::events::SDPAEvent::Ptr)
  {
	std::clog << "transmission of message failed!" << std::endl;
  }
};

int main(int argc, char **argv)
{
  fhg::log::Configurator::configure();
  seda::comm::initialize(argc, argv);

  int errcount(0);
  {
	typedef seda::comm::delivery_service<sdpa::events::SDPAEvent::Ptr, sdpa::events::SDPAEvent::message_id_type, seda::Stage> sdpa_msg_delivery_service;

    seda::StageFactory sFactory;
    seda::Strategy::Ptr discard(new seda::DiscardStrategy("discard"));
	seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    seda::Stage::Ptr stage(sFactory.createStage("final", ecs));

	seda::comm::ServiceThread service_thread;
	sdpa_msg_delivery_service service(service_thread.io_service(), 500);

	sdpa_msg_delivery_service::callback_handler h;
	h = handler();
	service.register_callback_handler(h);

	stage->start();
	service.start();
	service_thread.start();

	// test successful send
	{
	  std::clog << "testing successful send of a single message...";
	  sdpa::events::SubmitJobEvent::Ptr sj(new sdpa::events::SubmitJobEvent("from", "to", "job-id-1", "job-desc", "parent-job"));
	  service.send(stage.get(), sj, sj->id(), 1, 1);

	  if (ecs->wait(1, 1000))
	  {
		service.acknowledge(sj->id());
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
