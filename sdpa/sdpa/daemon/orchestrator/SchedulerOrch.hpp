/*
 * =====================================================================================
 *
 *       Filename:  SchedulerOrch.hpp
 *
 *    Description:  The orchestrator's scheduler
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

#ifndef SDPA_SCHEDULERORCH_HPP
#define SDPA_SCHEDULERORCH_HPP 1

#include <sdpa/daemon/SchedulerImpl.hpp>

using namespace sdpa::events;
using namespace std;

namespace sdpa {
        namespace daemon {
  class SchedulerOrch : public SchedulerImpl {

  public:
         SchedulerOrch(sdpa::daemon::IAgent* pCommHandler = NULL,  bool bUseReqModel = true):
                 SchedulerImpl(pCommHandler, bUseReqModel),
                 SDPA_INIT_LOGGER(pCommHandler?pCommHandler->name()+"::Scheduler":"Scheduler")
         {}

         virtual ~SchedulerOrch()
         {
			try
			{
				stop();
			}
			catch (std::exception const & ex)
			{
				SDPA_LOG_ERROR("could not stop SchedulerOrch: " << ex.what());
			}
         }

         bool post_request( bool ) { return false; }
         void send_life_sign() { /*do nothing*/ }
         void check_post_request() { /*do nothing*/ }

         template <class Archive>
         void serialize(Archive& ar, const unsigned int)
         {
           ar & boost::serialization::base_object<SchedulerImpl>(*this);
         }

         friend class boost::serialization::access;
         //friend class sdpa::tests::WorkerSerializationTest;

         bool has_job(const sdpa::job_id_t& job_id)
         {
			if( pending_jobs_queue_.find(job_id) != pending_jobs_queue_.end() )
			{
				SDPA_LOG_INFO("The job "<<job_id<<" is still in the jobs_to_be_scheduled queue!");
				return true;
			}

			return ptr_worker_man_->has_job(job_id);
         }

  private:
         SDPA_DECLARE_LOGGER();
  };
}}

#endif
