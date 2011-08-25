/*
 * =====================================================================================
 *
 *       Filename:  SchedulerNRE.hpp
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
#ifndef SDPA_SchedulerNRE_HPP
#define SDPA_SchedulerNRE_HPP 1

#include <sdpa/daemon/SchedulerImpl.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>

using namespace sdpa::events;
using namespace std;

namespace sdpa {
  namespace daemon {
    template <typename U>
    class SchedulerNRE : public SchedulerImpl {
    public:
      SchedulerNRE( sdpa::daemon::IComm* pHandler = NULL,
    		  	  std::string workerUrl = ""
                  // TODO: fixme, this is ugly
                  , bool bLaunchNrePcd = false
                  , const std::string & fvmPCBinary = ""
                  , const std::vector<std::string> & fvmPCSearchPath = std::vector<std::string>()
                  , const std::vector<std::string> & fvmPCPreLoad = std::vector<std::string>()
                  , bool bUseReqModel = true
                  )
        : sdpa::daemon::SchedulerImpl(pHandler, bUseReqModel)
        , SDPA_INIT_LOGGER((pHandler?"Scheduler "+pHandler->name():"Scheduler NRE"))
        , m_worker_(workerUrl, bLaunchNrePcd, fvmPCBinary, fvmPCSearchPath, fvmPCPreLoad)
      {
        m_worker_.set_ping_interval(5);
        m_worker_.set_ping_timeout(10);
        m_worker_.set_ping_trials(3);
      }

      virtual ~SchedulerNRE()
      {
        try
        {
          LOG(TRACE, "destructing SchedulerNRE");
          stop();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not stop SchedulerNRE: " << ex.what());
        }
      };

      void start(IComm* p)
      {
    	  SDPA_LOG_DEBUG("Starting NreWorkerClient ...");
    	  try {
    		  ptr_comm_handler_->rank() = m_worker_.start();
    	  }
    	  catch(const std::exception& val)
    	  {
    	      SDPA_LOG_ERROR("Could not start the nre-worker-client: " << val.what());
    	      throw;
    	  }

    	  SchedulerImpl::start(p);
      }

	void stop()
	{
            LOG(TRACE, "Stopping nre scheduler...");
            SchedulerImpl::stop();

            LOG(TRACE, "Stopping nre worker...");
            m_worker_.stop();
	}

	 /*void check_post_request()
	 {
             if( ptr_comm_handler_->is_registered() )
             {
                 //SDPA_LOG_DEBUG("Check if a new request is to be posted");
                 // post job request if number_of_jobs() < #registered workers + 1
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
	 }*/

	 virtual void execute(const sdpa::job_id_t& jobId) throw (std::exception)
	 {
             MLOG(TRACE, "executing activity: "<< jobId);
             const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);
             id_type act_id = pJob->id().str();

             execution_result_t result;
             encoded_type enc_act = pJob->description(); // assume that the NRE's workflow engine encodes the activity!!!

             if( !ptr_comm_handler_ )
             {
                 LOG(ERROR, "nre scheduler does not have a comm-handler!");
                 result_type output_fail;
                 ptr_comm_handler_->notifyActivityFailed(act_id, enc_act);
                 ptr_comm_handler_->workerJobFailed("", jobId, output_fail);
                 return;
             }

            try
            {
                ptr_comm_handler_->notifyActivityStarted(act_id, enc_act);
                pJob->Dispatch();
                result = m_worker_.execute(enc_act, pJob->walltime());
            }
            catch( const boost::thread_interrupted &)
            {
                std::string errmsg("could not execute activity: interrupted");
                SDPA_LOG_ERROR(errmsg);
                result = std::make_pair(ACTIVITY_FAILED, enc_act);
            }
            catch (const std::exception &ex)
            {
                std::string errmsg("could not execute activity: ");
                errmsg += std::string(ex.what());
                SDPA_LOG_ERROR(errmsg);
                result = std::make_pair(ACTIVITY_FAILED, enc_act);
            }

            // check the result state and invoke the NRE's callbacks
            if( result.first == ACTIVITY_FINISHED )
            {
                DLOG(TRACE, "activity finished: " << act_id);
                // notify the gui
                // and then, the workflow engine
                //ptr_comm_handler_->notifyActivityFinished(act_id, enc_act);
                ptr_comm_handler_->notifyActivityFinished(act_id, result.second);
                ptr_comm_handler_->workerJobFinished("", jobId, result.second);
            }
            else if( result.first == ACTIVITY_FAILED )
            {
                DLOG(TRACE, "activity failed: " << act_id);
                // notify the gui
                // and then, the workflow engine
                ptr_comm_handler_->notifyActivityFailed(act_id, enc_act);
                ptr_comm_handler_->workerJobFailed("", jobId, result.second);
            }
            else if( result.first == ACTIVITY_CANCELLED )
            {
                DLOG(TRACE, "activity cancelled: " << act_id);

                // notify the gui
                // and then, the workflow engine
                ptr_comm_handler_->notifyActivityCancelled(act_id, enc_act);
                ptr_comm_handler_->workerJobCancelled("", jobId);
            }
            else
            {
                SDPA_LOG_ERROR("Invalid status of the executed activity received from the NRE worker!");
                ptr_comm_handler_->notifyActivityFailed(act_id, enc_act);
                ptr_comm_handler_->workerJobFailed("", jobId, result.second);
            }
	 }

	 void schedule_remote(const sdpa::job_id_t& /*jobId*/)
	 {
	     throw std::runtime_error ("Schedule remote not implemented for the NREs!");
	 }

	 void run()
	 {
            if(!ptr_comm_handler_)
            {
                SDPA_LOG_FATAL("The scheduler cannot be started. Invalid communication handler. ");
                stop();
                return;
            }

            SDPA_LOG_DEBUG("Scheduler thread running ...");

            while(!bStopRequested)
            {
                try
                {
                    if(useRequestModel())
                      check_post_request();

                    sdpa::job_id_t jobId = jobs_to_be_scheduled.pop_and_wait(m_timeout);
                    const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

                    if( !pJob->is_local() )
                    {
                        try {
                            DLOG(TRACE, "Try to execute the job "<<jobId.str()<<" ...");
                            execute(jobId);
                        }
                        catch(JobNotFoundException& ex)
                        {
                            SDPA_LOG_DEBUG("Job not found! Could not schedule locally the job "<<ex.job_id().str());
                        }
                        catch(const NoWorkerFoundException&)
                        {
                            // put the job back into the queue
                            jobs_to_be_scheduled.push(jobId);
                            SDPA_LOG_DEBUG("Cannot schedule the job. No worker available! Put the job back into the queue.");
                        }
                    }
                    else
                        schedule_local(jobId);

                }
                catch(JobNotFoundException const & ex)
                {
                    SDPA_LOG_DEBUG("Job not found! Could not schedule locally the job "<<ex.job_id().str());
                }
                catch( const boost::thread_interrupted & )
                {
                    DMLOG(DEBUG, "Thread interrupted ...");
                    bStopRequested = true; // FIXME: can probably be removed
                    break;
                }
                catch( const sdpa::daemon::QueueEmpty &)
                {
                  // ignore
                }
                catch ( const std::exception &ex )
                {
                    MLOG(ERROR, "exception in scheduler thread: " << ex.what());
                }
            }
	 }

	 void print()
	 {
            if(!jobs_to_be_scheduled.empty())
            {
                SDPA_LOG_DEBUG("The content of agent's scheduler queue is:");
                jobs_to_be_scheduled.print();
            }
            else
                SDPA_LOG_DEBUG("No job to be scheduled left!");
	 }

	 friend class boost::serialization::access;
	 //friend class sdpa::tests::WorkerSerializationTest;

	 template <class Archive>
	 void serialize(Archive& ar, const unsigned int)
	 {
	     ar & boost::serialization::base_object<SchedulerImpl>(*this);
	     //ar & m_worker_;  //NreWorkerClient
	 }

  private:
	  SDPA_DECLARE_LOGGER();
	  U m_worker_;
  };
}}

#endif
