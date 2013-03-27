/*
 * =====================================================================================
 *
 *       Filename:  SchedulerAgg.hpp
 *
 *    Description:  The aggregator's scheduler
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
#ifndef SDPA_SCHEDULERAGG_HPP
#define SDPA_SCHEDULERAGG_HPP 1

#include <sdpa/daemon/SchedulerImpl.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>

using namespace sdpa::events;
using namespace std;

namespace sdpa {
	namespace daemon {
  class AgentScheduler : public SchedulerImpl {

  public:
	 AgentScheduler(sdpa::daemon::IAgent* pCommHandler = NULL,  bool use_request_model=true):
	   SchedulerImpl(pCommHandler, use_request_model),
	   SDPA_INIT_LOGGER(pCommHandler?pCommHandler->name()+"::Scheduler":"Scheduler")
	{
	}

	 virtual ~AgentScheduler()
	 {
		 try
		 {
			 LOG(TRACE, "destructing SchedulerAgg");
			 stop();
		 }
		 catch (std::exception const & ex)
		 {
			 LOG(ERROR, "could not stop SchedulerAgg: " << ex.what());
		 }
	 }

	 friend class boost::serialization::access;

	 template <class Archive>
	 void serialize(Archive& ar, const unsigned int /* file_version */)
	 {
		 ar & boost::serialization::base_object<SchedulerImpl>(*this);
	 }

  private:
	  SDPA_DECLARE_LOGGER();
  };
}}

#endif
