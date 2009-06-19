#ifndef SDPA_MGMT_EVENT_HPP
#define SDPA_MGMT_EVENT_HPP 1

#include <string>

#include <sdpa/memory.hpp>
#include <sdpa/events/SDPAEvent.hpp>

namespace sdpa {
namespace events {
    class MgmtEvent : public sdpa::events::SDPAEvent {
    public:
        typedef sdpa::shared_ptr<MgmtEvent> Ptr;

        MgmtEvent(const address_t &from, const address_t &to)
          : SDPAEvent(to, from) {}
        ~MgmtEvent() {}
    };
}}

#endif // SDPA_JOB_EVENT_HPP
