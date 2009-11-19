/*
 * =====================================================================================
 *
 *       Filename:  IComm.hpp
 *
 *    Description:  Interface used for communication
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
	  virtual void sendEvent(seda::Stage* ptrOutStage, const sdpa::events::SDPAEvent::Ptr& e)=0;
	  virtual void sendEvent(const sdpa::events::SDPAEvent::Ptr& e)=0;
	  virtual seda::Stage* to_master_stage() const = 0;
	  virtual seda::Stage* to_slave_stage() const = 0;

	  virtual void jobFinished(std::string workerName, const job_id_t &)=0;
	  virtual void jobFailed(std::string workerName, const job_id_t &)=0;
	  virtual void jobCancelled(std::string workerName, const job_id_t &)=0;

	  virtual void activityFinished(	const gwes::workflow_id_t &wf_id
									  , const gwes::activity_id_t &act_id
									  , const sdpa::parameter_list_t &output)
	  { gwes()->activityFinished(wf_id, act_id, output); }

	  // only for testing with DummyGwes, change it
	  virtual sdpa::Sdpa2Gwes* gwes() const = 0;
	  virtual std::string master()const = 0;
	  virtual const std::string& name() const = 0;
	  virtual bool is_registered() const = 0;
	  virtual sdpa::util::Config* cfg() const = 0;
  };
}}

#endif
