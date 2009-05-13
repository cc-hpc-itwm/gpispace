#ifndef SDPA_MESSAGES_CANCELJOBEVENT_HPP
#define SDPA_MESSAGES_CANCELJOBEVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa {
  namespace events {
    class CancelJobEvent : public sdpa::events::JobEvent {
      public:
        typedef std::tr1::shared_ptr<CancelJobEvent> Ptr;
        CancelJobEvent(){}
        ~CancelJobEvent(){}
    };
  }
}


#endif
