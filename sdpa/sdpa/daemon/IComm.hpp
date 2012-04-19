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

#include <sdpa/daemon/mpl.hpp>
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
  class JobManager;

  class IComm{
  public:
    virtual ~IComm() {}

	  virtual void sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& e) = 0;
	  virtual void sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& e) = 0;
	  virtual void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& e) = 0;
	  virtual void requestRegistration() = 0;
	  virtual void requestRegistration(const MasterInfo& masterInfo) = 0;
 	  virtual void requestJob(const MasterInfo& masterInfo) = 0;

	  virtual JobManager::ptr_t jobManager() const = 0;

	  virtual const Worker::worker_id_t& findWorker(const sdpa::job_id_t& job_id) const = 0;
	  virtual Job::ptr_t& findJob(const sdpa::job_id_t& job_id ) const = 0;
	  virtual void deleteJob(const sdpa::job_id_t& ) = 0;

	  virtual const requirement_list_t getJobRequirements(const sdpa::job_id_t& jobId) const = 0;

	  virtual void submitWorkflow(const id_type & id, const encoded_type & ) = 0;
	  virtual void cancelWorkflow(const id_type& workflowId, const std::string& reason) = 0;

	  virtual void activityFailed(const Worker::worker_id_t& worker_id, const job_id_t&, const std::string& result /*or reason*/ ) = 0;
	  virtual void activityFinished(const Worker::worker_id_t& worker_id, const job_id_t & id, const result_type& result ) = 0;
	  virtual void activityCancelled(const Worker::worker_id_t& worker_id, const job_id_t& id ) = 0;

	  virtual const std::string& name() const = 0;
	  //virtual bool is_registered() const = 0;
	  virtual sdpa::util::Config& cfg() = 0;

	  virtual unsigned int& rank() = 0;
	  virtual const sdpa::worker_id_t& agent_uuid() = 0;

	  virtual void updateLastRequestTime() = 0;
	  virtual bool requestsAllowed() = 0;

	  virtual void serveJob(const Worker::worker_id_t& worker_id, const job_id_t& last_id = sdpa::JobId::invalid_job_id()) = 0;

	  virtual void schedule(const sdpa::job_id_t& job) = 0;

	  virtual bool hasWorkflowEngine() = 0;
	  virtual bool isTop() = 0;

	  virtual void backup( std::ostream& ) { throw std::runtime_error("not supported at this level"); }
	  virtual void recover( std::istream& ) { throw std::runtime_error("not supported at this level"); }

	  virtual bool isScheduled(const sdpa::job_id_t& job_id) = 0;

	  //GUI notification methods
	  virtual void notifyActivityCreated(const id_type&, const std::string& )   { throw std::runtime_error("not supported by this component"); }
	  virtual void notifyActivityStarted(const id_type&, const std::string& )   { throw std::runtime_error("not supported by this component"); }
	  virtual void notifyActivityFinished(const id_type&, const std::string& )  { throw std::runtime_error("not supported by this component"); }
	  virtual void notifyActivityFailed(const id_type&, const std::string& )    { throw std::runtime_error("not supported by this component"); }
	  virtual void notifyActivityCancelled(const id_type&, const std::string& ) { throw std::runtime_error("not supported by this component"); }

	  virtual sdpa::master_info_list_t& getListMasterInfo() = 0;
	  virtual void getCapabilities(sdpa::capabilities_set_t& cpbset) = 0;
	  virtual void addCapability(const capability_t&) = 0;

	  virtual bool canRunTasksLocally() { return false; }
  };
}}

#endif
