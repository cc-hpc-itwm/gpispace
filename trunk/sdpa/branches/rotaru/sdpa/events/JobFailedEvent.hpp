#ifndef SDPA_MESSAGES_JOBFAILEDEVENT_HPP
#define SDPA_MESSAGES_JOBFAILEDEVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa {
  namespace events {
    class JobFailedEvent : public sdpa::events::JobEvent {
      public:
        typedef std::tr1::shared_ptr<JobFailedEvent> Ptr;
        JobFailedEvent(){}
        ~JobFailedEvent(){}
    };
  }
}


#endif
