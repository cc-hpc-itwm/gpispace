#ifndef SDPA_MGMT_EVENT_HPP
#define SDPA_MGMT_EVENT_HPP 1

#include <string>

#include <sdpa/memory.hpp>
#include <sdpa/events/SDPAEvent.hpp>

namespace sdpa { namespace events {
  class MgmtEvent : public sdpa::events::SDPAEvent {
    public:
      typedef sdpa::shared_ptr<MgmtEvent> Ptr;

      MgmtEvent()
        : SDPAEvent()
      {}
      MgmtEvent(const address_t &a_from, const address_t &a_to)
        : SDPAEvent(a_from, a_to) {}

      int priority() const { return 1; }

      virtual std::string str() const = 0;
  };
}}

#endif // SDPA_MGMT_EVENT_HPP
