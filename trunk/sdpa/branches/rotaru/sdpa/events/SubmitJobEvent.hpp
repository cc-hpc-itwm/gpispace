#ifndef SDPA_MESSAGES_SUBMITJOBEVENT
#define SDPA_MESSAGES_SUBMITJOBEVENT 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa {
  namespace events {
    class SubmitJobEvent : public sdpa::events::JobEvent {
      public:
        typedef std::tr1::shared_ptr<SubmitJobEvent> Ptr;
        SubmitJobEvent(){}
        ~SubmitJobEvent(){}
    };
  }
}


#endif
