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

#include <sdpa/daemon/IWorkflowEngine.hpp>

#include <seda/Stage.hpp>
#include <sdpa/util/Config.hpp>
#include <sdpa/types.hpp>
#include <sdpa/daemon/JobManager.hpp>
#include <sdpa/daemon/Worker.hpp>

#define MSG_RETRY_CNT 5

namespace sdpa { namespace daemon {

const std::string ORCHESTRATOR("orchestrator") ;
const std::string AGGREGATOR("aggregator") ;
//const std::string NRE("NRE");
const std::string WE("WE");
const std::string USER("user");

  class IComm{
  public:
	  virtual void sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& e, std::size_t retries = 0, unsigned long timeout = 1) = 0;
	  virtual void sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& e, std::size_t retries = 0, unsigned long timeout = 1) = 0;
	  virtual void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& e)=0;
	  virtual bool acknowledge(const sdpa::events::SDPAEvent::message_id_type &mid) = 0;

	  virtual const Worker::worker_id_t& findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException) = 0;
	  virtual Job::ptr_t& findJob(const sdpa::job_id_t& job_id ) throw (JobNotFoundException) = 0;
	  virtual const we::preference_t& getJobPreferences(const sdpa::job_id_t& jobId) const throw (NoJobPreferences) = 0;

	  virtual void submitWorkflow(const id_type & id, const encoded_type & ) throw (NoWorkflowEngine) = 0;
	  virtual void cancelWorkflow(const id_type& workflowId, const std::string& reason) = 0;

	  virtual void workerJobFailed(const job_id_t&, const std::string& result /*or reason*/ ) = 0;
	  virtual void workerJobFinished(const job_id_t & id, const result_type& result ) = 0;
	  virtual void workerJobCancelled(const job_id_t& id ) = 0;

	  virtual std::string master()const = 0;
	  virtual const std::string& name() const = 0;
	  virtual bool is_registered() const = 0;
	  virtual sdpa::util::Config* cfg() const = 0;

	  virtual unsigned int& rank() = 0;
	  virtual bool requestsAllowed() = 0;

	  //GUI notification methods
	  virtual void activityCreated(const id_type& id, const std::string& data)   { throw std::runtime_error("not supported in this component"); }
	  virtual void activityStarted(const id_type& id, const std::string& data)   { throw std::runtime_error("not supported in this component"); }
	  virtual void activityFinished(const id_type& id, const std::string& data)  { throw std::runtime_error("not supported in this component"); }
	  virtual void activityFailed(const id_type& id, const std::string& data)    { throw std::runtime_error("not supported in this component"); }
	  virtual void activityCancelled(const id_type& id, const std::string& data) { throw std::runtime_error("not supported in this component"); }
  };
}}

#endif
