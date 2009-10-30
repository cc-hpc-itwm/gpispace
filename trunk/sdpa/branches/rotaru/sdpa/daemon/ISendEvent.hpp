#ifndef SDPA_I_SEND_EVENT_HANDLER_HPP
#define SDPA_I_SEND_EVENT_HANDLER_HPP 1

#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/memory.hpp>

#include <gwes/Gwes2Sdpa.h>
#include <sdpa/Sdpa2Gwes.hpp>

#include <seda/Stage.hpp>

#include <sdpa/Config.hpp>

namespace sdpa { namespace daemon {

const std::string ORCHESTRATOR("orchestrator") ;
const std::string AGGREGATOR("aggregator") ;
const std::string NRE("NRE") ;
const std::string USER("user") ;

  class IComm{
  public:
	  virtual void sendEvent(seda::Stage* ptrOutStage, const sdpa::events::SDPAEvent::Ptr& e)=0;
	  virtual void sendEvent(const sdpa::events::SDPAEvent::Ptr& e)=0;
	  virtual seda::Stage* to_master_stage() const = 0;
	  virtual seda::Stage* to_slave_stage() const = 0;

	  // only for testing with DummyGwes, change it
	  virtual sdpa::Sdpa2Gwes* gwes() const = 0;
	  virtual std::string master()const = 0;
	  virtual const std::string& name() const = 0;
	  virtual bool is_registered() const = 0;
	  virtual sdpa::config::Config* cfg() const = 0;
  };
}}

#endif
