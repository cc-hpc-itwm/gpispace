#ifndef SDPA_MESSAGES_SUBMITJOBACKEVENT
#define SDPA_MESSAGES_SUBMITJOBACKEVENT 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa {
  namespace events {
    class SubmitJobAckEvent : public sdpa::events::JobEvent {
      public:
        typedef std::tr1::shared_ptr<SubmitJobAckEvent> Ptr;
        SubmitJobAckEvent(){}
        ~SubmitJobAckEvent(){}
    };
  }
}


#endif
