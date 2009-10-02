#ifndef SDPA_I_SEND_EVENT_HANDLER_HPP
#define SDPA_I_SEND_EVENT_HANDLER_HPP 1

#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/memory.hpp>
#include <sdpa/wf/Sdpa2Gwes.hpp>

namespace sdpa { namespace daemon {
  class IComm{
  public:
	  virtual void sendEvent(const std::string& stageName, const sdpa::events::SDPAEvent::Ptr& e)=0;
	  virtual const std::string output_stage() const = 0;
	  virtual sdpa::wf::Sdpa2Gwes* gwes() const = 0;
	  virtual std::string master()const = 0;
	  virtual const std::string& name() const = 0;
  };
}}

#endif
