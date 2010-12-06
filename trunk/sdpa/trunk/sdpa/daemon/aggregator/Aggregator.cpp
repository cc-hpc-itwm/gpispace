/*
 * =====================================================================================
 *
 *       Filename:  Aggreagtor.hpp
 *
 *    Description:  Contains the Aggregator class
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
#include <sdpa/daemon/aggregator/Aggregator.hpp>
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;


Aggregator::~Aggregator()
{
	SDPA_LOG_DEBUG("Aggregator destructor called ...");
	daemon_stage_ = NULL;
}


void Aggregator::start(Aggregator::ptr_t ptrAgg)
{
	dsm::DaemonFSM::create_daemon_stage(ptrAgg);
	ptrAgg->configure_network( ptrAgg->url(), ptrAgg->masterName() );
	sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
	ptrAgg->configure(ptrCfg);
}


void Aggregator::shutdown(Aggregator::ptr_t ptrAgg)
{
	ptrAgg->shutdown_network();
	ptrAgg->stop();

	if(ptrAgg->hasWorkflowEngine())
	{
		delete ptrAgg->ptr_workflow_engine_;
		ptrAgg->ptr_workflow_engine_ = NULL;
	}
}


void Aggregator::action_configure(const StartUpEvent &se)
{
	GenericDaemon::action_configure (se);

	// should be overriden by the orchestrator, aggregator and NRE
	ptr_daemon_cfg_->put("nmax_ext_job_req", 10U);
	SDPA_LOG_INFO("Configuring myeself (aggregator)...");
}


void Aggregator::action_config_ok(const ConfigOkEvent& e)
{
        GenericDaemon::action_config_ok (e);

	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_INFO("Configuration (aggregator) was ok");
	{
	  std::ostringstream sstr;
	  ptr_daemon_cfg_->writeTo (sstr);
	  SDPA_LOG_INFO("config: " << sstr.str());
	}

	SDPA_LOG_INFO("Aggregator (" << name() << ") sending registration event to master (" << master() << ")");
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(name(), master(), rank()));
        sendEventToMaster (pEvtWorkerReg);
}


void Aggregator::action_interrupt(const InterruptEvent&)
{
	SDPA_LOG_DEBUG("Call 'action_interrupt'");
	// save the current state of the system .i.e serialize the daemon's state

}


void Aggregator::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
{
	assert (pEvt);

	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

	DLOG(TRACE, "handleJobFinished(" << pEvt->job_id() << ")");

	// TODO: WORK HERE refactor all this
	if (pEvt->from() != sdpa::daemon::WE)
	{
		// send a JobFinishedAckEvent back to the worker/slave
		JobFinishedAckEvent::Ptr pEvtJobFinishedAckEvt
		(new JobFinishedAckEvent( name()
								, pEvt->from()
								, pEvt->job_id()
								, pEvt->id()
								)
		);
	  // send the event to the slave
	  sendEventToSlave(pEvtJobFinishedAckEvt);
	}

	//put the job into the state Finished
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->JobFinished(pEvt);
	}
	catch(JobNotFoundException const &)
	{
          SDPA_LOG_WARN( "got finished message for old/unknown Job "<< pEvt->job_id());
          return;
	}

	if( pEvt->from() == sdpa::daemon::WE || !hasWorkflowEngine() ) // use a predefined variable here of type enum or use typeid
	{
		statistics::dump_maps();
		statistics::reset_maps();

		try {
			// forward it up
			JobFinishedEvent::Ptr pEvtJobFinished
                          (new JobFinishedEvent( name()
                                               , master()
                                               , pEvt->job_id()
                                               , pEvt->result()
                                               )
                          );

			// send the event to the master
			sendEventToMaster(pEvtJobFinished);
		}
		catch(QueueFull const &)
		{
			SDPA_LOG_ERROR("Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a JobFinishedEvent");
		}
		catch(seda::StageNotFound const &)
		{
			SDPA_LOG_ERROR("Stage not found when trying to submit JobFinishedEvent");
		}
		catch(std::exception const & ex) {
                  SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
                  throw;
		}
		catch(...) {
                  SDPA_LOG_FATAL("Unexpected exception occurred!");
                  throw;
		}
	}
	else //event sent by a worker
	{
		Worker::worker_id_t worker_id = pEvt->from();

		try {
                  id_type actId = pEvt->job_id();

                  result_type output = pEvt->result();

                  // this  should only  be called  once, therefore
                  // the state machine when we switch the job from
                  // one state  to another, the  code belonging to
                  // exactly    that    transition    should    be
                  // executed. I.e. all this code should go to the
                  // FSM callback routine.
                  if( hasWorkflowEngine() )
                  {
                	  SDPA_LOG_DEBUG("Inform WE that the activity "<<actId<<" finished");
                	  ptr_workflow_engine_->finished(actId, output);
                  }

                  try {
                	  SDPA_LOG_DEBUG("Remove the job "<<actId<<" from the worker"<<worker_id);
                	  ptr_scheduler_->deleteWorkerJob( worker_id, pJob->id() );
                  }
                  catch(WorkerNotFoundException const &)
                  {
                    SDPA_LOG_WARN("Worker "<<worker_id<<" not found!");
                    throw;
                  }
                  catch(const JobNotDeletedException&)
                  {
                    SDPA_LOG_ERROR("Could not delete the job "<<pJob->id()<<" from the worker "<<worker_id<<"'s queues ...");
                  }

                  if( hasWorkflowEngine() )
                  {
					  try {
						//delete it also from job_map_
						ptr_job_man_->deleteJob(pEvt->job_id());
					  }
					  catch(JobNotDeletedException const &)
					  {
						SDPA_LOG_ERROR("The JobManager could not delete the job "<<pEvt->job_id());
						throw;
					  }
                  }
                }
                catch(std::exception const & ex) {
                  SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
		}
                catch(...)
                {
                  SDPA_LOG_FATAL("Unexpected exception occurred!");
                  throw;
                }
        }
}


void Aggregator::handleJobFailedEvent(const JobFailedEvent* pEvt )
{
	assert (pEvt);

	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

	DLOG(TRACE, "handleJobFailed(" << pEvt->job_id() << ")");

	// TODO: WORK HERE refactor all this
	if (pEvt->from() != sdpa::daemon::WE)
	{
		// send a JobFinishedAckEvent back to the worker/slave
		JobFailedAckEvent::Ptr evt(new JobFailedAckEvent( name()
							  , pEvt->from()
							  , pEvt->job_id()
							  , pEvt->id()
							  ));
		// send the event to the slave
		sendEventToSlave(evt);
	}

	//put the job into the state Finished
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->JobFailed(pEvt);
	}
	catch(JobNotFoundException const &){
		SDPA_LOG_WARN("Job "<<pEvt->job_id()<<" not found!");
		return;
	}

	if( pEvt->from() == sdpa::daemon::WE || !hasWorkflowEngine() ) // use a predefined variable here of type enum or use typeid
	{
		// the message comes from GWES
		try {
			// forward it up
			JobFailedEvent::Ptr pEvtJobFailedEvent(new JobFailedEvent(name(), master(), pEvt->job_id(), pEvt->result()));

			// send the event to the master
			sendEventToMaster(pEvtJobFailedEvent);
		}
		catch(QueueFull const &)
		{
			SDPA_LOG_ERROR("Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a JobFailedEvent");
		}
		catch(seda::StageNotFound const &)
		{
			SDPA_LOG_ERROR("Stage not found when trying to submit JobFailedEvent");
		}
		catch(std::exception const & ex) {
			SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
            throw;
		}
		catch(...)
		{
			SDPA_LOG_ERROR("Unexpected exception occurred!");
			throw;
		}

	}
	else //event sent by a worker
	{
		Worker::worker_id_t worker_id = pEvt->from();

		try {
			id_type actId = pJob->id().str();

			result_type output = pEvt->result();

			if( hasWorkflowEngine() )
			{
				SDPA_LOG_DEBUG("Inform WE that the activity "<<actId<<" failed");
				ptr_workflow_engine_->failed(actId, output);
			}

			try {
				SDPA_LOG_DEBUG("Remove the job "<< pJob->id()<<" from the worker"<<worker_id);
				ptr_scheduler_->deleteWorkerJob(worker_id, pJob->id());
			}
			catch(WorkerNotFoundException const &)
			{
				SDPA_LOG_ERROR("Worker "<<worker_id<<" not found!");
			}
			catch(const JobNotDeletedException&)
			{
				SDPA_LOG_ERROR("Could not delete the job "<<pJob->id()<<" from the worker "<<worker_id<<"'s queues ...");
			}

			if( hasWorkflowEngine() )
			{
				try {
					//delete it also from job_map_
					ptr_job_man_->deleteJob(pEvt->job_id());
				}catch(JobNotDeletedException const &){
					SDPA_LOG_ERROR("The JobManager could not delete the job "<<pEvt->job_id());
				}
			}
		}
		catch(std::exception const & ex)
		{
			SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
            throw;
		}
        catch(...)
        {
        	SDPA_LOG_ERROR("Unexpected exception occurred!");
            throw;
		}
	}
}


void Aggregator::handleCancelJobEvent(const CancelJobEvent* pEvt )
{
	assert (pEvt);

	LOG(INFO, "cancelling job: " << pEvt->job_id());

	// put the job into the state Cancelling
	try {
	  Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
	  pJob->CancelJob(pEvt);
	  DLOG(TRACE, "The job state is: "<<pJob->getStatus());
	}
	catch(JobNotFoundException const &){
	  SDPA_LOG_ERROR ("job " << pEvt->job_id() << " could not be found!");
	  throw;
	}
}


void Aggregator::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
{
	assert (pEvt);

	LOG(TRACE, "cancel acknowledgement received: " << pEvt->job_id());

	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

	// transition from Cancelling to Cancelled

	Worker::worker_id_t worker_id = pEvt->from();
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());

		// put the job into the state Cancelled
	    pJob->CancelJobAck(pEvt);
	    DLOG(TRACE, "The job state is: "<<pJob->getStatus());

    	// should send acknowlwdgement
    	if( pEvt->from() == sdpa::daemon::WE || !hasWorkflowEngine() ) // forward the message to the master
		{
			CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), master(), pEvt->job_id(), pEvt->id()));

			// only if the job was already submitted, send ack to master
			sendEventToMaster(pCancelAckEvt);

			// if I'm not the orchestrator, delete effectively the job
			ptr_job_man_->deleteJob(pEvt->job_id());

		}
    	else // the message comes from an worker, forward it to workflow engine
    	{
    		try {
    			SDPA_LOG_DEBUG("Remove the job "<< pJob->id()<<" from the worker"<<worker_id);
				ptr_scheduler_->deleteWorkerJob(worker_id, pJob->id());
    		}
    		catch(WorkerNotFoundException const &)
    		{
    			SDPA_LOG_ERROR("worker " << worker_id << " could not be found!");
    		}
    		catch(const JobNotDeletedException&)
			{
				SDPA_LOG_ERROR("Could not delete the job "<<pJob->id()<<" from the worker "<<worker_id<<"'s queues ...");
			}

    		// tell WE that the activity was cancelled
    		id_type actId = pJob->id();
    		if( hasWorkflowEngine() )
    		{
    			SDPA_LOG_DEBUG("Inform WE that the activity "<<actId<<" was cancelled");
    			ptr_workflow_engine_->cancelled(actId);

				try {
					//delete it also from job_map_
					ptr_job_man_->deleteJob(pEvt->job_id());
				}catch(JobNotDeletedException const &){
					SDPA_LOG_ERROR("The JobManager could not delete the job "<<pEvt->job_id());
				}
			}
    	}
	}
	catch(JobNotFoundException const &)
	{
		SDPA_LOG_ERROR("job could not be found: " << pEvt->job_id());
	}
	catch(JobNotDeletedException const &)
	{
		SDPA_LOG_ERROR("Job Manager could not delete job: " << pEvt->job_id());
	}
	catch (std::exception const &ex)
	{
		SDPA_LOG_ERROR("unexpected exception during cancel of: " << pEvt->job_id() << ": " << ex.what());
	}
	catch(...)
	{
		SDPA_LOG_ERROR("unexpected exception during cancel of: " << pEvt->job_id());
	}
}


void Aggregator::backup( const std::string& strArchiveName )
{
	try
	{
		print();

		Aggregator* ptrOrch(this);
		std::ofstream ofs( strArchiveName.c_str() );
		boost::archive::text_oarchive oa(ofs);
		oa.register_type(static_cast<Aggregator*>(NULL));
		oa.register_type(static_cast<DaemonFSM*>(NULL));
		oa.register_type(static_cast<GenericDaemon*>(NULL));
		oa.register_type(static_cast<SchedulerImpl*>(NULL));
		oa.register_type(static_cast<JobFSM*>(NULL));
		oa << ptrOrch;
	}
	catch(exception &e)
	{
		cout <<"Exception occurred: "<< e.what() << endl;
		return;
	}
}


void Aggregator::recover( const std::string& strArchiveName )
{
	try
	{
		Aggregator* pRestoredOrch(this);
		std::ifstream ifs( strArchiveName.c_str() );
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<Aggregator*>(NULL));
		ia.register_type(static_cast<DaemonFSM*>(NULL));
		ia.register_type(static_cast<GenericDaemon*>(NULL));
		ia.register_type(static_cast<SchedulerImpl*>(NULL));
		ia.register_type(static_cast<JobFSM*>(NULL));
		ia >> pRestoredOrch;

		print();
	}
	catch(exception &e)
	{
		cout <<"Exception occurred: " << e.what() << endl;
	}
}

