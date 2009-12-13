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
#ifndef SDPA_I_SEND_EVENT_HANDLER_HPP
#define SDPA_I_SEND_EVENT_HANDLER_HPP 1

#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/memory.hpp>

#include <gwes/Gwes2Sdpa.h>
#include <sdpa/Sdpa2Gwes.hpp>

#include <seda/Stage.hpp>
#include <sdpa/util/Config.hpp>
#include <sdpa/types.hpp>

#include <gwes/IActivity.h>

namespace sdpa { namespace daemon {

const std::string ORCHESTRATOR("orchestrator") ;
const std::string AGGREGATOR("aggregator") ;
const std::string NRE("NRE");
const std::string GWES("GWES");
const std::string USER("user");

  class IComm{
  public:
	  virtual void sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& e)=0;
	  virtual void sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& e)=0;
	  virtual void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& e)=0;

	  virtual void jobFinished(std::string workerName, const job_id_t &)=0;
	  virtual void jobFailed(std::string workerName, const job_id_t &)=0;
	  virtual void jobCancelled(std::string workerName, const job_id_t &)=0;

	  // only for testing with DummyGwes, change it
	  virtual sdpa::Sdpa2Gwes* gwes() const = 0;
	  virtual std::string master()const = 0;
	  virtual const std::string& name() const = 0;
	  virtual bool is_registered() const = 0;
	  virtual sdpa::util::Config* cfg() const = 0;

	  //GUI notification methods
	  virtual void activityCreated(const gwes::activity_t&)   { throw std::runtime_error("not supported in this component"); }
	  virtual void activityStarted(const gwes::activity_t&)   { throw std::runtime_error("not supported in this component"); }
	  virtual void activityFinished(const gwes::activity_t&)  { throw std::runtime_error("not supported in this component"); }
	  virtual void activityFailed(const gwes::activity_t&)    { throw std::runtime_error("not supported in this component"); }
	  virtual void activityCancelled(const gwes::activity_t&) { throw std::runtime_error("not supported in this component"); }
  };
}}

#endif
