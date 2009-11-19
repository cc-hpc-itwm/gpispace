#include <sdpa/daemon/SchedulerImpl.hpp>
/*
 * =====================================================================================
 *
 *       Filename:  SchedulerImpl.hpp
 *
 *    Description:  Implements a simple scheduler
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
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>

using namespace sdpa::daemon;
using namespace sdpa::events;
using namespace std;

SchedulerImpl::SchedulerImpl(sdpa::daemon::IComm* pCommHandler) :
	ptr_worker_man_(new WorkerManager()),
	ptr_comm_handler_(pCommHandler),
	SDPA_INIT_LOGGER(pCommHandler->name() + "::SchedulerImpl")
{
	m_timeout = boost::posix_time::milliseconds(1000);//time_duration(0,0,0,10);
	m_last_request_time = 0;
}

SchedulerImpl::~SchedulerImpl()
{
	SDPA_LOG_DEBUG("Called the destructor of  SchedulerImpl ...");
}

/*
	Schedule a job locally, send the job to GWES
*/
void SchedulerImpl::schedule_local(const Job::ptr_t &pJob) {
	SDPA_LOG_DEBUG("Called schedule_local ...");

	gwes::workflow_id_t wf_id = pJob->id().str();
	gwes::workflow_t* ptrWorkflow = NULL;

	// Use gwes workflow here!
	// IBuilder should be invoked here instead of this!!!
	try {
		ptrWorkflow = ptr_comm_handler_->gwes()->deserializeWorkflow( pJob->description() ) ;
		ptrWorkflow->setID(wf_id);
	} catch(std::runtime_error&){
		SDPA_LOG_ERROR("GWES could not deserialize the job description!"<<std::endl<<pJob->description());
		return;
	}

	// Should set the workflow_id here, or send it together with the workflow description
	ostringstream os;
	os<<"Submit the workflow attached to the job "<<wf_id<<" to GWES";
	SDPA_LOG_DEBUG(os.str());

	try {
		if(ptr_comm_handler_->gwes() && ptrWorkflow )
		{
			pJob->Dispatch();
			ptr_comm_handler_->gwes()->submitWorkflow(*ptrWorkflow);
		}
		else
			SDPA_LOG_ERROR("Gwes not initialized or workflow not created!");
	}
	catch (std::exception& )
	{
		SDPA_LOG_DEBUG("Exception occured when trying to submit the workflow "<<wf_id<<" to GWES!");
	}
}

/*
 * Implement here in a first phase a simple round-robin schedule
 */
void SchedulerImpl::schedule_remote(const Job::ptr_t &pJob) {
	SDPA_LOG_DEBUG("Called schedule_remote ...");

	try {

		if( ptr_worker_man_ )
		{
			SDPA_LOG_DEBUG("Get the next worker ...");
			Worker::ptr_t& pWorker = ptr_worker_man_->getNextWorker();

			SDPA_LOG_DEBUG("The job "<<pJob->id()<<" was assigned to the worker '"<<pWorker->name()<<"'!");
			pWorker->dispatch(pJob);
		}
	}
	catch(NoWorkerFoundException&)
	{
		// put the job back into the queue
		jobs_to_be_scheduled.push(pJob);
		SDPA_LOG_DEBUG("Cannot schedule the job. No worker available! Put the job back into the queue.");
	}
}

void SchedulerImpl::start_job(const Job::ptr_t &) {}

void SchedulerImpl::schedule(Job::ptr_t& pJob)
{
	ostringstream os;
	os<<"Handle job "<<pJob->id();
	SDPA_LOG_DEBUG(os.str());

	jobs_to_be_scheduled.push(pJob);
}

Worker::ptr_t &SchedulerImpl::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
	try {
		return ptr_worker_man_->findWorker(worker_id);
	}
	catch(WorkerNotFoundException)
	{
		throw WorkerNotFoundException(worker_id);
	}
}

void SchedulerImpl::addWorker(const Worker::ptr_t &pWorker)
{
	ptr_worker_man_->addWorker(pWorker);
}


void SchedulerImpl::start()
{
   bStopRequested = false;
   m_thread = boost::thread(boost::bind(&SchedulerImpl::run, this));
   SDPA_LOG_DEBUG("Scheduler thread started ...");
}

void SchedulerImpl::stop()
{
   m_thread.interrupt();

   bStopRequested = true;
   SDPA_LOG_DEBUG("Scheduler thread before join ...");
   m_thread.join();

   SDPA_LOG_DEBUG("Scheduler thread joined ...");
}

bool SchedulerImpl::post_request(bool force)
{
	bool bReqPosted = false;
	sdpa::util::time_type current_time = sdpa::util::now();
	sdpa::util::time_type difftime = current_time - m_last_request_time;

	if(force || (sdpa::daemon::ORCHESTRATOR != ptr_comm_handler_->name() &&  ptr_comm_handler_->is_registered()) )
	{
		if( difftime > ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("polling interval") )
		{
			// post a new request to the master
			// the slave posts a job request
			SDPA_LOG_DEBUG("Post a new request to "<<ptr_comm_handler_->master());
			RequestJobEvent::Ptr pEvtReq( new RequestJobEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master() ) );
			ptr_comm_handler_->sendEvent(ptr_comm_handler_->to_master_stage(), pEvtReq);
			m_last_request_time = current_time;
			bReqPosted = true;
		}
	}

	return bReqPosted;
}

void SchedulerImpl::send_life_sign()
{
	 sdpa::util::time_type current_time = sdpa::util::now();
	 sdpa::util::time_type difftime = current_time - m_last_request_time;

	 if( sdpa::daemon::ORCHESTRATOR != ptr_comm_handler_->name() &&  ptr_comm_handler_->is_registered() )
	 {
		 if( difftime > ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("life-sign interval") )
		 {
			 LifeSignEvent::Ptr pEvtLS( new LifeSignEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master() ) );
			 ptr_comm_handler_->sendEvent(ptr_comm_handler_->to_master_stage(), pEvtLS);
			 m_last_request_time = current_time;
		 }
	 }
}


void SchedulerImpl::check_post_request()
{
	 if( sdpa::daemon::ORCHESTRATOR != ptr_comm_handler_->name() &&  ptr_comm_handler_->is_registered() )
	 {
		 //SDPA_LOG_DEBUG("Check if a new request is to be posted");
		 // post job request if number_of_jobs() < #registered workers +1
		 if( jobs_to_be_scheduled.size() <= numberOfWorkers() + 3)
			 post_request();
		 else //send a LS
			 send_life_sign();
	 }
}


void SchedulerImpl::run()
{
	SDPA_LOG_DEBUG("Scheduler thread running ...");

	while(!bStopRequested)
	{
		try
		{
			//Job::ptr_t pJob = jobs_to_be_scheduled.pop_and_wait();
			Job::ptr_t pJob = jobs_to_be_scheduled.pop_and_wait(m_timeout);

			if( pJob.use_count() )
			{
				if(pJob->is_local())
					schedule_local(pJob);
				else
				{
					// if it's an NRE just execute it!
					// Attention!: an NRE has no WorkerManager!!!!
					// or has an Worker Manager and the workers are threads
					schedule_remote(pJob);
				}

				// if I'm not the orchestrator (i.e. either aggregator or nre)
				// if the job queue's length is less than twice the number of workers
				// and the elapased time since the last request is > polling_interval_time
			}

			check_post_request();
		}
		catch( const boost::thread_interrupted & )
		{
			SDPA_LOG_DEBUG("Thread interrupted ...");
			bStopRequested = true;
		}
		catch( const sdpa::daemon::QueueEmpty &)
		{
			//SDPA_LOG_DEBUG("Queue empty exception");
			check_post_request();
		}
	}
}
