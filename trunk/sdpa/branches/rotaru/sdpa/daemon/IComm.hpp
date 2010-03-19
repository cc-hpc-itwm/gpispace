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

	  virtual void jobFinished(std::string workerName, const job_id_t &)=0;
	  virtual void jobFailed(std::string workerName, const job_id_t &)=0;
	  virtual void jobCancelled(std::string workerName, const job_id_t &)=0;

	  // only for testing with DummyWorkflowEngine, change it
	  virtual IWorkflowEngine* workflowEngine() const = 0;
	  virtual std::string master()const = 0;
	  virtual const std::string& name() const = 0;
	  virtual bool is_registered() const = 0;
	  virtual sdpa::util::Config* cfg() const = 0;

	  virtual JobManager::ptr_t jobManager() const = 0;

	  //GUI notification methods
	  /*virtual void activityCreated(const gwes::activity_t&)   { throw std::runtime_error("not supported in this component"); }
	  virtual void activityStarted(const gwes::activity_t&)   { throw std::runtime_error("not supported in this component"); }
	  virtual void activityFinished(const gwes::activity_t&)  { throw std::runtime_error("not supported in this component"); }
	  virtual void activityFailed(const gwes::activity_t&)    { throw std::runtime_error("not supported in this component"); }
	  virtual void activityCancelled(const gwes::activity_t&) { throw std::runtime_error("not supported in this component"); }*/

  };
}}

#endif
