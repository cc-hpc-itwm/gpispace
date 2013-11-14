/*
 * =====================================================================================
 *
 *       Filename:  IAgent.hpp
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
#include <sdpa/daemon/scheduler/Reservation.hpp>

namespace sdpa {
  namespace daemon {
  const std::string WE("WE");

  class IAgent
  {
    public:
    virtual ~IAgent() {}

    virtual void sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& e) = 0;
    virtual void sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& e) = 0;
    virtual void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& e) = 0;
    virtual void requestRegistration() = 0;
    virtual void requestRegistration(const MasterInfo& masterInfo) = 0;

    virtual const sdpa::worker_id_t& findWorker(const sdpa::job_id_t& job_id) const = 0;
    virtual Job::ptr_t& findJob(const sdpa::job_id_t& job_id ) const = 0;
    virtual void deleteJob(const sdpa::job_id_t& ) = 0;
    virtual bool hasJobs() = 0;

    virtual const job_requirements_t getJobRequirements(const sdpa::job_id_t& jobId) const = 0;

    virtual void submitWorkflow(const id_type & id, const encoded_type & ) = 0;

    virtual void pause(const job_id_t& id ) = 0;
    virtual void resume(const job_id_t& id ) = 0;

    virtual const std::string& name() const = 0;
    virtual sdpa::util::Config& cfg() = 0;

    virtual unsigned int& rank() = 0;
    virtual const sdpa::worker_id_t& agent_uuid() = 0;

    virtual void serveJob(const sdpa::worker_id_t&, const job_id_t&) = 0;
    virtual void serveJob(const sdpa::worker_id_list_t& worker_list, const job_id_t& jobId) = 0;

    virtual void schedule(const sdpa::job_id_t& job) = 0;
    virtual bool hasWorkflowEngine() = 0;
    virtual bool isTop() = 0;

    virtual bool isScheduled(const sdpa::job_id_t& job_id) = 0;

    virtual sdpa::master_info_list_t& getListMasterInfo() = 0;
    virtual void getCapabilities(sdpa::capabilities_set_t& cpbset) = 0;
    virtual void addCapability(const capability_t&) = 0;
  };

}}

BOOST_SERIALIZATION_ASSUME_ABSTRACT( sdpa::daemon::IAgent )

#endif
