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
  class SchedulerAgg : public SchedulerImpl {

  public:
	 SchedulerAgg(sdpa::daemon::IComm* pCommHandler = NULL):
		 SchedulerImpl(pCommHandler),
		 SDPA_INIT_LOGGER("Scheduler " + (pCommHandler?pCommHandler->name():"AGG"))
	{

	}

	 virtual ~SchedulerAgg() {};

	 bool post_request(bool force = false)
	 {
		DMLOG(TRACE, "post request: force=" << force);
	 	bool bReqPosted = false;
	 	sdpa::util::time_type current_time = sdpa::util::now();
	 	sdpa::util::time_type difftime = current_time - m_last_request_time;

	 	if( force || ( difftime > ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("polling interval") &&
					   ptr_comm_handler_->requestsAllowed()) )
	 	{
	 		// post a new request to the master
	 		// the slave posts a job request
                  DMLOG(TRACE, "Post a new request to "<<ptr_comm_handler_->master());
	 		RequestJobEvent::Ptr pEvtReq( new RequestJobEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master() ) );
	 		ptr_comm_handler_->sendEventToMaster(pEvtReq);
	 		m_last_request_time = current_time;
	 		bReqPosted = true;
	 	}
		else
		{
		  DMLOG(TRACE, "not polling, difftime=" << difftime << " interval=" << ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("polling interval"));
		}

	 	return bReqPosted;
	 }

	 void send_life_sign()
	 {
	 	 sdpa::util::time_type current_time = sdpa::util::now();
	 	 sdpa::util::time_type difftime = current_time - m_last_life_sign_time;

	 	 if( ptr_comm_handler_->is_registered() )
	 	 {
	 		 if( difftime > ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("life-sign interval") )
	 		 {
				DMLOG(DEBUG, "sending life-sign to: " << ptr_comm_handler_->master());
	 			 LifeSignEvent::Ptr pEvtLS( new LifeSignEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master() ) );
	 			 ptr_comm_handler_->sendEventToMaster(pEvtLS);
	 			 m_last_life_sign_time = current_time;
	 		 }
	 	 }
		 else
		 {
		  DMLOG(DEBUG, "not sending life-sign, i am not registered yet");
		 }
	 }


	 void check_post_request()
	 {
	 	 if( ptr_comm_handler_->is_registered() )
	 	 {
	 		 //SDPA_LOG_DEBUG("Check if a new request is to be posted");
	 		 // post job request if number_of_jobs() < #registered workers +1
	 		 if( !post_request() )
				 //send a LS
	 			 send_life_sign();
	 	 }
		 else
		 {
		  DMLOG(DEBUG, "not requesting job, i am not registered yet");
		 }
	 }


	friend class boost::serialization::access;
	friend class sdpa::tests::WorkerSerializationTest;

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
