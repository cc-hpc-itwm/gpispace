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
	 		ptr_comm_handler_->requestJob();
	 		//SDPA_LOG_DEBUG("The agent "<<ptr_comm_handler_->name()<<" has posted a new job request!");

	 		update_request_time(current_time);
	 		bReqPosted = true;
	 	}

	 	return bReqPosted;
	 }

	 void check_post_request()
	 {
	 	 if( ptr_comm_handler_->is_registered() )
	 	 {
	 		 // post job request if number_of_jobs() < #registered workers +1
	 		 post_request();
	 	 }
	 	 else // try to re-register
			 {
	 		 	 SDPA_LOG_INFO("Try to re-register ...");
	 		 	 const unsigned long reg_timeout( ptr_comm_handler_->cfg().get<unsigned long>("registration_timeout", 1 *1000*1000) );
	 		 	 SDPA_LOG_INFO("Wait " << reg_timeout/1000000 << "s before trying to re-register ...");
	 		 	 boost::this_thread::sleep(boost::posix_time::microseconds(reg_timeout));

	 		 	 ptr_comm_handler_->requestRegistration();
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
