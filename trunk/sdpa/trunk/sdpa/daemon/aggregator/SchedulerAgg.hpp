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

	 virtual ~SchedulerAgg()
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

	 bool post_request(bool force = false)
	 {
		//DMLOG(TRACE, "post request: force=" << force);
	 	bool bReqPosted = false;
	 	sdpa::util::time_type current_time = sdpa::util::now();
	 	sdpa::util::time_type diff_time = current_time - m_last_request_time;

	 	if( force || ptr_comm_handler_->requestsAllowed(diff_time) )
	 	{
	 		// post a new request to the master
	 		// the slave posts a job request
            MLOG(TRACE, "Post a new request to "<<ptr_comm_handler_->master());
	 		RequestJobEvent::Ptr pEvtReq( new RequestJobEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master() ) );
	 		ptr_comm_handler_->sendEventToMaster(pEvtReq);

	 		update_request_time(current_time);
	 		bReqPosted = true;
	 	}

	 	return bReqPosted;
	 }

	 void check_post_request()
	 {
	 	 if( ptr_comm_handler_->is_registered() )
	 	 {
	 		 //SDPA_LOG_DEBUG("Check if a new request is to be posted");
	 		 // post job request if number_of_jobs() < #registered workers +1
	 		 post_request();
	 	 }
		 else
		 {
			 // send worker registration event
			 SDPA_LOG_INFO("Aggregator (" << ptr_comm_handler_->name() << ") sending registration event to master (" << ptr_comm_handler_->master() << ")");
			 WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(ptr_comm_handler_->name(), ptr_comm_handler_->master(),
					                                                                ptr_comm_handler_->rank(), ptr_comm_handler_->agent_uuid()));
			 ptr_comm_handler_->sendEventToMaster(pEvtWorkerReg);

			 // use here a registration time-out !!!!!!!!!!!!!!!!!!!
			 sleep(1);
		 }
	 }


	friend class boost::serialization::access;
	//friend class sdpa::tests::WorkerSerializationTest;

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
