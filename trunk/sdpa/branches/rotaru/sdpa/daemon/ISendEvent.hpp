#ifndef SDPA_I_SEND_EVENT_HANDLER_HPP
#define SDPA_I_SEND_EVENT_HANDLER_HPP 1

#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/memory.hpp>

namespace sdpa { namespace daemon {
  class ISendEvent{
  public:
	  virtual void sendEvent(const std::string& stageName, const sdpa::events::SDPAEvent::Ptr& e)=0;

  };
}}

#endif
