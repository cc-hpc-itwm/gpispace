#ifndef SDPA_MESSAGES_CANCELJOBACKEVENT_HPP
#define SDPA_MESSAGES_CANCELJOBACKEVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa {
  namespace events {
    class CancelJobAckEvent : public sdpa::events::JobEvent {
      public:
        typedef std::tr1::shared_ptr<CancelJobAckEvent> Ptr;
        CancelJobAckEvent() {}
        ~CancelJobAckEvent() {}
    };
  }
}


#endif
