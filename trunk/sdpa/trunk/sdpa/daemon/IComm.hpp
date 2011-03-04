/*
 * =====================================================================================
 *
 *       Filename:  IComm.hpp
 *
 *    Description:  Interface used for communication
 *
 *        Version:  1.0
 *        Created:  2004
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef SDPA_ICOMM_HPP
#define SDPA_ICOMM_HPP 1

#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/memory.hpp>

#include <sdpa/engine/IWorkflowEngine.hpp>

#include <seda/Stage.hpp>
#include <sdpa/util/Config.hpp>
#include <sdpa/types.hpp>
#include <sdpa/daemon/JobManager.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/JobId.hpp>

namespace sdpa { namespace daemon {

const std::string ORCHESTRATOR("orchestrator") ;
const std::string AGGREGATOR("aggregator") ;
//const std::string NRE("NRE");
const std::string WE("WE");
const std::string USER("user");

  class JobNotDeletedException;

  class IComm{
  public:
    virtual ~IComm() {}

	  virtual void sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& e) = 0;
	  virtual void sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& e) = 0;
	  virtual void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& e) = 0;
	  virtual void requestRegistration() = 0;
	  virtual void requestJob() = 0;

	  virtual const Worker::worker_id_t& findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException) = 0;
	  virtual Job::ptr_t& findJob(const sdpa::job_id_t& job_id ) throw (JobNotFoundException) = 0;
	  virtual void deleteJob(const sdpa::job_id_t& ) throw(JobNotDeletedException) = 0;

	  virtual const preference_t& getJobPreferences(const sdpa::job_id_t& jobId) const throw (NoJobPreferences) = 0;

	  virtual void submitWorkflow(const id_type & id, const encoded_type & ) = 0;
	  virtual void cancelWorkflow(const id_type& workflowId, const std::string& reason) = 0;

	  virtual void workerJobFailed(const Worker::worker_id_t& worker_id, const job_id_t&, const std::string& result /*or reason*/ ) = 0;
	  virtual void workerJobFinished(const Worker::worker_id_t& worker_id, const job_id_t & id, const result_type& result ) = 0;
	  virtual void workerJobCancelled(const Worker::worker_id_t& worker_id, const job_id_t& id ) = 0;

	  virtual std::string master()const = 0;
	  virtual const std::string& name() const = 0;
	  virtual bool is_registered() const = 0;
	  virtual sdpa::util::Config* cfg() const = 0;

	  virtual unsigned int& rank() = 0;
	  virtual const sdpa::worker_id_t& agent_uuid() = 0;
	  virtual bool requestsAllowed(const sdpa::util::time_type&) = 0;
	  virtual void schedule(const sdpa::job_id_t& job) = 0;

	  virtual bool hasWorkflowEngine() = 0;
	  virtual bool is_orchestrator() = 0;

	  virtual void backup( std::ostream& ) { throw std::runtime_error("not supported at this level"); }
	  virtual void recover( std::istream& ) { throw std::runtime_error("not supported at this level"); }

	  virtual bool is_scheduled(const sdpa::job_id_t& job_id) = 0;

	  //GUI notification methods
	  virtual void notifyActivityCreated(const id_type&, const std::string& )   { throw std::runtime_error("not supported by this component"); }
	  virtual void notifyActivityStarted(const id_type&, const std::string& )   { throw std::runtime_error("not supported by this component"); }
	  virtual void notifyActivityFinished(const id_type&, const std::string& )  { throw std::runtime_error("not supported by this component"); }
	  virtual void notifyActivityFailed(const id_type&, const std::string& )    { throw std::runtime_error("not supported by this component"); }
	  virtual void notifyActivityCancelled(const id_type&, const std::string& ) { throw std::runtime_error("not supported by this component"); }
  };
}}

#endif
